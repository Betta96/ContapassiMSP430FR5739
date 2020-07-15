/*
 * myfunctions.h
 *
 *  Created on: 12 lug 2020
 *      Author: betta_000
 */

#ifndef MYFUNCTIONS_H_
#define MYFUNCTIONS_H_

void setup_clk();                                   // impostare clock
void setup_adc();                                   // impostare ADC
void setup_S1();                                    //impostare switch 1
void setup_S2();                                    //impostare switch 2
void setup_external_button();                       //impostare switch esterno

void DisableSwitches();                             //disabilita l'interrupt per i tasti S1 e S2
void EnableSwitches();                              //ri-abilita l'interrupt per i tasti S1 e S2
void DisableExternalSwitch();                       //disabilita l'interrupt per il tasto esterno
void EnableExternalSwitch();                        //ri-abilita l'interrupt per il tasto esterno
void enable_acc();                                  //abilita l'accelerometro

void start_conversion();                            //fa partire la conversione

int getLunghezza(int number);                       //ritorna la lunghezza (# cifre) del numero in ingresso

void adc_to_mV(volatile int arr[],volatile int valore);               //trasforma il valore in uscita dall'ADC in mV, prendendo in ingresso il valore da trasformare e un array in cui mettere
                                                    //in posizione 0 il valore convertito prima della virgola (parte intera) e in posizione 1 quello dopo la virgola (parte decimale)

long calibrazioneX (void);                          //ritorna la calibrazione dell'asse x
long calibrazioneY (void);                          //ritorna la calibrazione dell'asse y
long calibrazioneZ (void);                          //ritorna la calibrazione dell'asse z

int calories (int peso, int metri);                 //ritorna il numero di calorie calcolate in base ai parametri d'ingresso (peso e distanza percorsa)
int distance (int altezza,int passi,char sesso);    //ritorna i metri percorsi in base ai parametri d'ingresso (altezza, sesso e passi)



#endif /* MYFUNCTIONS_H_ */
