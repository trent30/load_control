#include <LiquidCrystal.h>

LiquidCrystal lcd(1, 0, 5, 4, 3, 2);

//~ R1 = 10k  ohms
//~ R2 = 2.2k ohms
//~ en théorie R = R1 / R2 … en pratique :
float R = 9.81; 

long bt_prog = A0;
long bt_up = A1;
long bt_down = A2;
long photoresistance1 = A3;
long photoresistance2 = A4;
long voltmetre = A5;
long value;
float v_input;
int bt_prog_delay = 2000;
int bt_delay = 300;
int bt_limit = 1000;
int limit_boucle_secu = 30;
int cpt_loop = 0;
int cpt_loop_limit = 30;		// le temps entre 2 mesures vaut (en ms):  t = bt_prog_delay * cpt_loop_limit
								// exemple : 30 * 2000 = 1 minute
int seuil_nuit = 300;
#define MAX 10
float tab[MAX]; 				// tableau utilisé pour le calcul de la moyenne lissée : MEAN
int nb_relais_in = 7;			// nombre de relais utilisés en entrée (pour les panneaux solaires)
int relais_on = 0;				// nombre de relais actuellement fermés
int FRIGO_PIN  = 6;
float MEAN = 12.0;
float V_IN_MAX = 13.1;			// Si la tension des batteries dépasse ce seuil alors on diminue nb_relais_in
float V_IN_MIN = 12.8;			// Si la tension des batteries est en dessous de ce seuil alors on augmente nb_relais_in
float FRIGO_V_MAX = 12.7;		// Si la tension des batteries dépasse ce seuil alors on démarre le frigo
float FRIGO_V_MIN = 12.5;		// Si la tension des batteries est en dessous de ce seuil alors on éteind le frigo

//~ SECURITY
float MEAN_MAX = 15.0;			// Au delà on coupe tout
float MEAN_MIN = 10.0;			// Au dessous on coupe tout


void display_tab_value(int name, float value) {
	lcd.clear();
	lcd.print(name);
	lcd.print(" : ");
	lcd.setCursor(0, 1);
	lcd.print(value);
}

void add_value(float v) {
	for ( int i = 0; i < MAX - 1 ; i++) {
		tab[i] = tab[i+1];
	}
	tab[MAX - 1] = v;
}

float moyenne() {
	float sum = 0;
	for ( int i = 0; i < MAX ; i++) {
		sum += tab[i];
	}
	return sum / MAX;
}

void switch_frigo(int v) {
	if (v == 1) {
		digitalWrite(FRIGO_PIN, LOW);
	} else {
		digitalWrite(FRIGO_PIN, HIGH);
	}
}

void update_relais() {
	for ( int i = 0; i < relais_on ; i++) {
		digitalWrite(13 - i, LOW);
	}
	for ( int i = relais_on; i < nb_relais_in ; i++) {
		digitalWrite(13 - i, HIGH);
	}
}

void COUPE_TOUT() {
	// On coupe le FRIGO
	switch_frigo(0);
	
	// Et on coupe les relais
	relais_on = 0;
	update_relais();	
}

void SECURITE(float m) {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("ALERT !");
	lcd.setCursor(0, 1);
	lcd.print("mean=");
	lcd.print(m);
	COUPE_TOUT();
}

void display_on_lcd() {
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(MEAN);
	lcd.print(" ");
	lcd.print(v_input);
	lcd.print(" I");
	lcd.print(long(cpt_loop_limit) * bt_prog_delay / 1000);
	lcd.print(" ");
	lcd.setCursor(0, 1);
	lcd.print(int(V_IN_MIN*10));
	lcd.print("/");
	lcd.print(int(V_IN_MAX*10));
	lcd.print("#");
	lcd.print(int(FRIGO_V_MAX*10));
	lcd.print("/");
	lcd.print(int(FRIGO_V_MIN*10));
}

void display_one_value(String name, float value) {
	lcd.clear();
	lcd.print(name);
	lcd.print(" :");
	lcd.setCursor(0, 1);
	lcd.print(value);
}

void display_seuil_nuit() {
	lcd.clear();
	lcd.print("NUIT : ");
	lcd.print(lecture_photoresistance());
	lcd.setCursor(0, 1);
	lcd.print(seuil_nuit);
}

void display_ratio() {
	lcd.clear();
	lcd.print("RATIO : ");
	lcd.print(R);
	v_input = analogRead(voltmetre);
	v_input = v_input * 5 * R / 1024;
	lcd.setCursor(0, 1);
	lcd.print(v_input);
	lcd.print("V");
}

int change_seuil_nuit() {
	value = 0;
	int secu = 0;
	display_seuil_nuit();
	delay(bt_delay);
	while (value < bt_limit) {
		if (analogRead(bt_up) > bt_limit) {
			seuil_nuit += 1;
		}
		if (analogRead(bt_down) > bt_limit) {
			seuil_nuit -= 1;
		}
		display_seuil_nuit();
		delay(bt_delay);
		value = analogRead(bt_prog);
		secu += 1;
		if (secu > limit_boucle_secu) {
			return 1;
		}
	}
	return 0;
}

int change_intervalle() {
	value = 0;
	int secu = 0;
	display_one_value("INTERVALLE (s)", long(cpt_loop_limit) * bt_prog_delay / 1000);
	delay(bt_delay);
	while (value < bt_limit) {
		if (analogRead(bt_up) > bt_limit) {
			cpt_loop_limit += 1;
		}
		if (analogRead(bt_down) > bt_limit) {
			cpt_loop_limit -= 1;
		}
		display_one_value("INTERVALLE", long(cpt_loop_limit) * bt_prog_delay / 1000);
		delay(bt_delay);
		value = analogRead(bt_prog);
		secu += 1;
		if (secu > limit_boucle_secu) {
			return 1;
		}
	}
	change_seuil_nuit();
	return 0;
}

int change_RATIO() {
	value = 0;
	int secu = 0;
	display_ratio();
	delay(bt_delay);
	while (value < bt_limit) {
		if (analogRead(bt_up) > bt_limit) {
			R += 0.01;
		}
		if (analogRead(bt_down) > bt_limit) {
			R -= 0.01;
		}
		display_ratio();
		delay(bt_delay);
		value = analogRead(bt_prog);
		secu += 1;
		if (secu > limit_boucle_secu * 10) {
			// x10 : 1 minute 30s au lieu de 9s 
			// laisse le temps de régler le bon ratio
			return 1;
		}
	}
	change_intervalle();
	return 0;
}

int change_FRIGO_V_MIN() {
	value = 0;
	int secu = 0;
	display_one_value("FRIGO_V_MIN", FRIGO_V_MIN);
	delay(bt_delay);
	while (value < bt_limit) {
		if (analogRead(bt_up) > bt_limit) {
			FRIGO_V_MIN += 0.1;
		}
		if (analogRead(bt_down) > bt_limit) {
			FRIGO_V_MIN -= 0.1;
		}
		display_one_value("FRIGO_V_MIN", FRIGO_V_MIN);
		delay(bt_delay);
		value = analogRead(bt_prog);
		secu += 1;
		if (secu > limit_boucle_secu) {
			return 1;
		}
	}
	change_RATIO();
	return 0;
}

int change_FRIGO_V_MAX() {
	value = 0;
	int secu = 0;
	display_one_value("FRIGO_V_MAX", FRIGO_V_MAX);
	delay(bt_delay);
	while (value < bt_limit) {
		if (analogRead(bt_up) > bt_limit) {
			FRIGO_V_MAX += 0.1;
		}
		if (analogRead(bt_down) > bt_limit) {
			FRIGO_V_MAX -= 0.1;
		}
		display_one_value("FRIGO_V_MAX", FRIGO_V_MAX);
		delay(bt_delay);
		value = analogRead(bt_prog);
		secu += 1;
		if (secu > limit_boucle_secu) {
			return 0;
		}
	}
	change_FRIGO_V_MIN();
	return 0;
}

int change_V_IN_MAX() {
	value = 0;
	int secu = 0;
	display_one_value("V_IN_MAX", V_IN_MAX);
	delay(bt_delay);
	while (value < bt_limit) {
		if (analogRead(bt_up) > bt_limit) {
			V_IN_MAX += 0.1;
		}
		if (analogRead(bt_down) > bt_limit) {
			V_IN_MAX -= 0.1;
		}
		display_one_value("V_IN_MAX", V_IN_MAX);
		delay(bt_delay);
		value = analogRead(bt_prog);
		secu += 1;
		if (secu > limit_boucle_secu) {
			return 0;
		}
	}
	change_FRIGO_V_MAX();
	return 0;
}

int change_V_IN_MIN() {
	value = 0;
	int secu = 0;
	display_one_value("V_IN_MIN", V_IN_MIN);
	delay(bt_delay);
	while (value < bt_limit) {
		if (analogRead(bt_up) > bt_limit) {
			V_IN_MIN += 0.1;
		}
		if (analogRead(bt_down) > bt_limit) {
			V_IN_MIN -= 0.1;
		}
		display_one_value("V_IN_MIN", V_IN_MIN);
		delay(bt_delay);
		value = analogRead(bt_prog);
		secu += 1;
		if (secu > limit_boucle_secu) {
			return 0;
		}
	}
	change_V_IN_MAX();
	return 0;
}

float mesure_v_input() {
	int nb_read = 50;
	float sum = 0;
	for ( int i = 0; i < nb_read; i++) {
		sum += analogRead(voltmetre);
		delay(1);
	}
	return sum * 5 * R / nb_read / 1024;
}

int lecture_photoresistance() {
	return analogRead(photoresistance1) + analogRead(photoresistance2);
}

boolean nuit() {
	if (lecture_photoresistance() > seuil_nuit) {
		return false;
	}
	return true;
}

void setup() {
	lcd.begin(16, 2);
	lcd.print("Initialisation...");
	for ( int i = 0; i < 8 ; i++) {
		// les pins 6 à 13 sont connectées aux relais
		pinMode(13 - i, OUTPUT);
	}
	switch_frigo(0);
	relais_on = nb_relais_in;
	update_relais();
	for ( int i = 0; i < MAX ; i++) {
		add_value(mesure_v_input());
	}
	cpt_loop = cpt_loop_limit;
}

void loop() {
	value = analogRead(bt_prog);
	if (value > bt_limit) {
		change_V_IN_MIN();
	}
	display_on_lcd();
	
	cpt_loop += 1;
	if (cpt_loop >= cpt_loop_limit) {
		cpt_loop = 0;
		
		if ( nuit() ) {
			COUPE_TOUT();
		} else {
			
			//~ LECTURE V_INPUT
			v_input = mesure_v_input();
			add_value(v_input);
			MEAN = moyenne();
			
			// check MEAN
			if (MEAN < MEAN_MIN || MEAN > MEAN_MAX) {
				SECURITE(MEAN);
			} else {
				// Si tout va bien on continue
				
				// check panneaux
				if (MEAN > V_IN_MAX && relais_on > 0) {
					relais_on -= 1;
				}
				if (MEAN < V_IN_MIN && relais_on < nb_relais_in) {
					relais_on += 1;
				}
				update_relais();
				
				// check frigo
				if (MEAN > FRIGO_V_MAX) {
					switch_frigo(1);
				}
				if (MEAN < FRIGO_V_MIN) {
					switch_frigo(0);
				}
				
			}
		}
	}
	delay(bt_prog_delay);
}

