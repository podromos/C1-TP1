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
			if(simu->CTS == 1){  // make sure that successive transmits cannot take place
				simu->RTS = 0;
			}
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
			if (simu->CTS==1){
				simu->RTS = 1;
				simu->state_reception = 0;
			}
	}
}

/* routine d'interruption */
void timer_interruption(struct simulator* simu){
	*(simu->broche_Tx)=simu->Tx;
	simu->Rx = *(simu->broche_Rx);
	*(simu->broche_RTS)=simu->RTS;
	simu->CTS=*(simu->broche_CTS);
	automate_reception_RTS_CTS(simu);
	automate_emission_RTS_CTS(simu);
}

/* MAIN */

void* main_simulator1(void* data){
	unsigned char var_to_send='a';
	struct simulator* simu=(struct simulator*) data;

	sleep(1);
	
	while (var_to_send <='z'){
		if (!simu->TRANSMIT){
			printf("simulator1 : envoie de %c\n",var_to_send);
			simu->var_Tx=var_to_send;
			simu->TRANSMIT=1;
			var_to_send++;
		}
	}

	return NULL;
}

void* main_simulator2(void* data){
	struct simulator* simu=(struct simulator*) data;
	
	while(1){
		if (simu->RECEIVED==1){
			printf("simulator2 : %c recu\n",simu->var_loc);
			simu->RECEIVED=0;
			if (simu->var_loc=='z')break;
		}
	}

	return NULL;
}




