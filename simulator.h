
struct simulator{
	/* adresse simulant les fils*/
	unsigned char* broche_Tx;
	unsigned char* broche_Rx;
	unsigned char* broche_RTS;
	unsigned char* broche_CTS;

	/*Variables de l'automate*/
	unsigned char Tx;
	unsigned char Rx;
	unsigned char RTS;
	unsigned char CTS;
	
	/*Variables de l'automate d'émission*/
	unsigned char var_Tx;
	unsigned char TRANSMIT; /* Flag indiquant la transmission de données*/
	unsigned char state_emission;
	unsigned char count_emission;
	unsigned char nb_bit_emission;

	unsigned char* array_Tx; /* pointeurs sur un tableau à envoyer */
	unsigned char index; /* indice de l'élement en cours d'envoie */
	unsigned char size; /* taille du tableau array_Tx */

	unsigned char TRANSMIT_XON_XOFF; /* Flag indiquant l'envoie d'un signal XON/XOFF */

	/*Variables de l'automates de réception*/
	unsigned char var_Rx;
	unsigned char RECEIVED;
	unsigned char state_reception;
	unsigned char count_reception;
	unsigned char nb_bit_reception;
	
	unsigned char var_loc; /* variable dans laquelle est stocckée un donnée une fois sa reception terminée */

	unsigned char XOFF; /*Flag de demande d'interruption */
};

/* routine d'interruption*/
void timer_interruption(struct simulator* simu);

/* fonction main executé par un simulator passé en paramètre*/
void* main_simulator1(void* data);

void* main_simulator2(void* data);