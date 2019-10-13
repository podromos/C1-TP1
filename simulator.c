#include <stdio.h>
#include <unistd.h>
#include "simulator.h"

/*PARTIE INTERESSANTE*/ 

/* AUTOMATES SIMPLES*/
void automate_emission(struct simulator* simu){
	switch(simu->state_emission){
		case 0:
			simu->Tx = 1;
			if(simu->TRANSMIT == 1)
			{
				simu->state_emission = 1;
			}
			break;

		case 1:
			simu->Tx = 0;
			simu->nb_bit_emission = 0;
			simu->count_emission = 0;
		   	simu->state_emission = 2;
			break;

		case 2:
			simu->count_emission++;
			if (simu->count_emission == 12)
			{
				if (simu->nb_bit_emission != 9)
					simu->state_emission = 3;
				else
					simu->state_emission = 4;
			}
			break;

		case 3:
			simu->count_emission = 0;
			if(simu->nb_bit_emission < 8)
			{
				simu->Tx = simu->var_Tx & ((char)0x01);
				simu->var_Tx = (simu->var_Tx) >> 1;
			}
			else
				simu->Tx=1;
			simu->nb_bit_emission++;
			simu->state_emission = 2;
			break;

		case 4:
			simu->TRANSMIT = 0;
			simu->state_emission = 0;
			break;
	}
}

void automate_reception(struct simulator* simu){
	switch(simu->state_reception)
	{
		case 0:
			simu->count_reception = 0;
			if(!simu->Rx)
				simu->state_reception = 1;
			break;

		case 1:
			simu->var_loc = 0x00;
			simu->count_reception++;
			if(simu->count_reception == 5)
				simu->state_reception = 2;
			break;

		case 2:
			simu->count_reception = 0;
			simu->nb_bit_reception = 0;
			simu->state_reception = 3;
			break;

		case 3:
			simu->count_reception++;
			if (simu->count_reception == 12)
			{
				if (simu->nb_bit_reception != 8)
					simu->state_reception = 4;
				else
					simu->state_reception = 5;
			}
			break;

		case 4:
			simu->count_reception = 0;
			simu->nb_bit_reception++;
			simu->var_Rx = (simu->var_Rx) >> 1;
			if(simu->Rx)
				simu->var_Rx = simu->var_Rx | 0x80;
			simu->state_reception = 3;
			break;

		case 5:
			if(simu->RECEIVED == 0)
	   		{
				simu->var_loc=simu->var_Rx;
				simu->RECEIVED = 1;
			}
			simu->state_reception = 0;
			break;
	}
}

/*AUTOMATES RTS/CTS*/

void automate_emission_RTS_CTS(struct simulator* simu){
	switch(simu->state_emission)
	{
		case 0:
			simu->Tx = 1;
			if(simu->TRANSMIT == 1)
				simu->state_emission = 1;
			break;

		case 1:
			/* On s'assure que le récepteur est en attente pour passer RTS à 1*/
			if(simu->CTS == 1){  
				simu->RTS = 0;
			}
			/* Si CTS et RTS = 0 on début l'envoie */
			else if(simu->RTS == 0){	
				simu->state_emission = 2;
			}
			break;

		case 2:
			simu->Tx = 0;
			simu->nb_bit_emission = 0;
			simu->count_emission = 0;
			simu->state_emission = 3;
			break;

		case 3:
			simu->count_emission++;
			if (simu->count_emission == 12)
			{
				if (simu->nb_bit_emission != 9)
					simu->state_emission = 4;
				else
					simu->state_emission = 5;
			}
			break;

		case 4:
			simu->count_emission = 0;
			if(simu->nb_bit_emission < 8)
			{
				simu->Tx = (simu->var_Tx) & ((char)0x01);
				simu->var_Tx = (simu->var_Tx) >> 1;
			}
			else
				simu->Tx = 1;
			simu->nb_bit_emission++;
			simu->state_emission = 3;
			break;

		case 5:
			simu->RTS = 1;
			simu->TRANSMIT = 0;
			simu->state_emission = 0;
			break;
	}
}

void automate_reception_RTS_CTS(struct simulator* simu){
	switch(simu->state_reception)
	{
		case 0:
			/* Si une demande d'envoie est faite*/
			if(simu->CTS == 0)
			{
				simu->RTS = 0;
				simu->state_reception = 1;
			}
			break;

		case 1:
			simu->count_reception = 0;
			if(!simu->Rx)
				simu->state_reception = 2;
			break;

		case 2:
			simu->var_Rx = 0x00;
			simu->count_reception++;
			if(simu->count_reception == 5)
				simu->state_reception = 3;
			break;

		case 3:
			simu->count_reception = 0;
			simu->nb_bit_reception = 0;
			simu->state_reception = 4;
			break;

		case 4:
			simu->count_reception++;
			if (simu->count_reception == 12)
			{
				if (simu->nb_bit_reception != 8)
					simu->state_reception = 5;
				else
					simu->state_reception = 6;
			}
			break;

		case 5:
			simu->count_reception = 0;
			simu->nb_bit_reception++;
			simu->var_Rx = (simu->var_Rx) >> 1;
			if(simu->Rx)
				simu->var_Rx = (simu->var_Rx) | 0x80;
			simu->state_reception = 4;
			break;

		case 6:
			if(simu->RECEIVED == 0)
	   		{
				simu->var_loc = simu->var_Rx;
				simu->RECEIVED = 1;
			}
			simu->state_reception=7;
			break;
		case 7:
			/* On attent l'envoie de l'octet soit déclarée terminée par l'émetteur (via CTS)*/
			if (simu->CTS==1){
				simu->RTS = 1;
				simu->state_reception = 0;
			}
	}
}


/*AUTOMATES XON/XOFF avec prise en charge de l'envoie de tableau*/

void automate_emission_XON_XOFF(struct simulator* simu){
	switch(simu->state_emission){
		case 0:
			simu->Tx = 1;
			if(simu->TRANSMIT || simu->TRANSMIT_XON_XOFF)
			{
				simu->state_emission = 1;
			}
			break;

		case 1:
			/* Si aucun signal XOFF n'a été reçu */
			if (!simu->XOFF ){
				/* Si l'émission en cours correspond à l'émission d'un tableau de donnée (et non d'un signal XON/XOFF)*/
				if(simu->TRANSMIT){
					simu->var_Tx=simu->array_Tx[simu->index];
				}
				simu->Tx = 0;
				simu->nb_bit_emission = 0;
				simu->count_emission = 0;
	   			simu->state_emission = 2;
	   		}
			break;

		case 2:
			simu->count_emission++;
			if (simu->count_emission == 12)
			{
				if (simu->nb_bit_emission != 9)
					simu->state_emission = 3;
				/* Si l'émission correspond à l'envoie d'un tableau de données */
				else if(simu->TRANSMIT) 
					simu->state_emission = 4;
				/* Si l'émission correspond à l'envoie d'un signal XON/XOFF */
				else
					simu->state_emission = 5;
			}
			break;

		case 3:
			simu->count_emission = 0;
			if(simu->nb_bit_emission < 8)
			{
				simu->Tx = simu->var_Tx & ((char)0x01);
				simu->var_Tx = (simu->var_Tx) >> 1;
			}
			else
				simu->Tx=1;
			simu->nb_bit_emission++;
			simu->state_emission = 2;
			break;

		case 4: 
			simu->index++;
			/* Si le dernier élément n'est pas atteint */
			/* Envoie de l'élément d'indice "index+1" du tableau */
			if (simu->index<simu->size){
				simu->state_emission = 1;
			}
			else{
				simu->TRANSMIT = 0;
				simu->index=0;	
				simu->state_emission = 0;
			}
			break;
		case 5:
			simu->TRANSMIT_XON_XOFF=0;
			simu->state_emission=0;
			break;
	}
}

void automate_reception_XON_XOFF(struct simulator* simu){
	switch(simu->state_reception)
	{
		case 0:
			simu->count_reception = 0;
			if(!simu->Rx)
				simu->state_reception = 1;
			break;

		case 1:
			simu->count_reception++;
			if(simu->count_reception == 5)
				simu->state_reception = 2;
			break;

		case 2:
			simu->count_reception = 0;
			simu->nb_bit_reception = 0;
			simu->state_reception = 3;
			break;

		case 3:
			simu->count_reception++;
			if (simu->count_reception == 12)
			{
				if (simu->nb_bit_reception != 8)
					simu->state_reception = 4;
				else
					simu->state_reception = 5;
			}
			break;

		case 4:
			simu->count_reception = 0;
			simu->nb_bit_reception++;
			simu->var_Rx = (simu->var_Rx) >> 1;
			if(simu->Rx)
				simu->var_Rx = simu->var_Rx | 0x80;
			simu->state_reception = 3;
			break;

		case 5:
			if(simu->RECEIVED == 0)
	   		{
	   			/*Si une émission d'un tableau de données est en cours */
	   			if (simu->TRANSMIT){
	   				/* Mise à jour si nécessaire du flag XOFF*/
	   				if(simu->var_Rx==17){
	   					simu->XOFF=0;
	   				}
	   				else if (simu->var_Rx==19){
	   					simu->XOFF=1;
	   				}
	   			}
	   			else{	
					simu->var_loc=simu->var_Rx;
					simu->RECEIVED = 1;
				}
			}
			simu->state_reception = 0;
			break;
	}
}

/* Envoie d'un tableau pour les automates de base et RTS/CTS*/
void send_array(struct simulator* simu,char* t,unsigned int size){
	unsigned int i=0;
	while (i<size){
		if (!simu->TRANSMIT){
			simu->var_Tx=t[i];
			simu->TRANSMIT=1;
			i++;
		}
	}
}

/* routine d'interruption */
void timer_interruption(struct simulator* simu){
	*(simu->broche_Tx)=simu->Tx;
	simu->Rx = *(simu->broche_Rx);
	*(simu->broche_RTS)=simu->RTS;
	simu->CTS=*(simu->broche_CTS);
	automate_reception_XON_XOFF(simu);
	automate_emission_XON_XOFF(simu);
}

/* MAIN */

/* exemple de l'envoie deux fois d'un tableau */
void* main_simulator1(void* data){
	
	struct simulator* simu=(struct simulator*) data;

	sleep(1);
	
	unsigned char array[10]={1,2,3,4,5,6,7,8,9,10};
	simu->array_Tx=array;
	simu->size=10;
	simu->TRANSMIT=1;
	while(simu->TRANSMIT);
	simu->array_Tx=array;
	simu->size=10;
	simu->TRANSMIT=1;
	while(1);
	return NULL;
}

/* Réception avec intérruption XON/XOFF */
void* main_simulator2(void* data){
	struct simulator* simu=(struct simulator*) data;
	
	while(1){
		if (simu->RECEIVED==1){
			printf("simulator2 : %d recu\n",simu->var_loc);
			simu->RECEIVED=0;
			if (simu->var_loc==2){
				simu->var_Tx=19;
				simu->TRANSMIT_XON_XOFF=1;
				printf("interruption\n");
			}
			if (simu->var_loc==3){
				sleep(5);
				simu->var_Tx=17;
				simu->TRANSMIT_XON_XOFF=1;
				printf("redémarrage\n");
			}
		}
	}

	return NULL;
}




