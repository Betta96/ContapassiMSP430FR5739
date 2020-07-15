/*
 * myfunctions.c
 *
 *  Created on: 12 lug 2020
 *      Author: betta_000
 */
#include <msp430.h>
#include "myfunctions.h"
#include "string.h"

/*IMPOSTAZIONE DEL CLOCK*/
void setup_clk(){
    CSCTL0 = 0xA500;                        //password per clk
    CSCTL1 |= DCORSEL;                      //DCO a 20MHz
    CSCTL1 |= DCOFSEL_2;                   //DCO a 20MHz
    CSCTL2 |= SELA_3 | SELS_3 | SELM_3;     //tutte le sorgenti di clock collegate al DCO
    CSCTL3 |= DIVA_4 | DIVS_0 | DIVM_0;     //divisore a 1 per SMCLK e MCLK, a 16 per ACLK --> ACLK = DCO/16 = 1.25MHz
    CSCTL0_H = 0;                           //blocco registri con password sbagliata in modalità byte (macro, riferire al registro mettendo _H o _L; _H 8bit alti _L quelli bassi)
    /*Si vuole un clock veloce, 20MHz, ma è troppo alta la frequenza per l'ADC, così si va oltre il limite di 200 ksample per s e ci saranno errori di overflow. Si opera con il
     *segnale di clock ACLK e prediviso con divisore 16, che verrà collegato all'ADC nella sua configurazione.*/
}

/*IMPOSTAZIONE DELL'ADC*/
void setup_adc(){
    ADC10CTL0 &= ~ADC10ENC;         //disabilita l'ADC (mettere a 0 il bit ENC, per forza per permettere di operare sui registri di configurazione)
    ADC10CTL0 |= ADC10SHT_15;       //si imposta il S&H su 1024 cicli di clock, è un S&H lungo
    ADC10CTL0 |= ADC10ON;           //si attiva l'alimentazione per dell'ADC
    ADC10CTL1 |= ADC10SSEL_1;       //si seleziona la sorgente di clock ACLK
    ADC10CTL1 |= ADC10CONSEQ_0;     //si seleziona la modalità di operazione come single-channel single-conversion
    ADC10CTL1 |= ADC10SHP  ;        //si seleziona la modalità di sampling come pulse-mode
    ADC10MCTL0 |= ADC10INCH_12;     //si imposta il canale di input sull'A12
    ADC10MCTL0 |= ADC10SREF_0;      //segnali di riferimento AVcc e AVss, è già così di default
    ADC10IE |= ADC10IE0;            //si abilita l'interrupt che definisce la fine della conversione
    P3SEL0 |= BIT0 | BIT1;          //operare sul pin 3.0 (ACC) con seconda funzione in modo da impostare P3.0 come input ADC
    ADC10IFG = 0;                   //reset interrupt flags
    ADC10CTL0 |= ADC10ENC;          //finite tutte le impostazioni si porta a 1 ENC per ri-abilitare l'ADC
    __delay_cycles(1000000);        //consente all'ADC di stabilizzarsi prima di convertire qualsiasi dato
}

/*IMPOSTAZIONE DELLO SWITCH S1*/
void setup_S1(){
    P4SEL1 &= ~BIT0;
    P4SEL0 &= ~BIT0;            //con queste due istruzioni si imposta P4.0 come GPIO
    P4DIR &= ~BIT0;             //si imposta P4.0 come input
    P4REN |= BIT0;              //si abilitano le resistenze di pull-up/down
    P4OUT |= BIT0;              //si seleziona il pull-up
    P4IE = BIT0;                //abilito gli interrupt relativi alla porta 4
    P4IFG = 0;                  //ripulisco il flag della porta 4

}

/*IMPOSTAZIONE DELLO SWITCH S2*/
void setup_S2(){
    P4SEL1 &= ~BIT1;
    P4SEL0 &= ~BIT1;            //con queste due istruzioni si imposta P4.1 come GPIO
    P4DIR &= ~BIT1;             //si imposta P4.1 come input
    P4REN |= BIT1;              //si abilitano le resistenze di pull-up/down
    P4OUT |= BIT1;              //si seleziona il pull-up
    P4IE = BIT1;                //abilito gli interrupt relativi alla porta P4.1
    P4IFG = 0;                  //ripulisco il flag della porta P4.1

}

/*IMPOSTAZIONE DELLO SWITCH ESTERNO (COLLEGATO AL PIN P1.1)*/
void setup_external_button(){
    P1SEL1 &= ~BIT1;
    P1SEL0 &= ~BIT1;            //con queste due istruzioni si imposta P1.1 come GPIO
    P1DIR &=~ BIT1;             //si imposta P1.1 come input
    P1REN |= BIT1;              //si abilitano le resistenze di pull-up/down
    P1OUT |= BIT1;              //si seleziona il pull-up
    P1IE |= BIT1;               //si abilitano gli interrupt relativi alla porta P1.1
    P1IFG = 0;                  //ripulisco il flag della porta P1.1
}

/*DISABILITA L'INTERRUPT DEGLI SWITCH S1 E S2*/
void DisableSwitches(){
  // disable switches
  P4IFG = 0;                                // Si ripulisce il flag della P4
  P4IE &= ~(BIT0 | BIT1);                   // si disabilitano gli interrupt relativi alle porte P4.0 e P4.1
  P4IFG = 0;                                // Si ripulisce il flag della P4
}

/*DISABILITA L'INTERRUPT DELLO SWITCH ESTERNO*/
void DisableExternalSwitch(){
  // disable switches
  P1IFG = 0;               // Si ripulisce il flag della P1
  P1IE &= ~(BIT1);         // si disabilitano gli interrupt relativi alla porta P1.1
  P1IFG = 0;               // Si ripulisce il flag della P1
}

/*RI-ABILITA L'INTERRUPT DEGLI SWITCH S1 E S2*/
void EnableSwitches(){
  P4IFG = 0;                 // Si ripulisce il flag della P4
  P4IE = BIT0 | BIT1;        // si abilitano gli interrupt relativi alle porte P4.0 e P4.1
}

/*RI-ABILITA L'INTERRUPT DELLO SWITCH ESTERNO*/
void EnableExternalSwitch(){
  P1IFG = 0;                 // Si ripulisce il flag della P1
  P1IE = BIT1;               // si abilitano gli interrupt relativi alla porta P1.1
}


/*ABILITAZIONE ACCELEROMETRO*/
void enable_acc(){
    P2DIR |= BIT7;              //P2.7 come output e messa a 1/livello alto, attiva l'alimentazione
    P2OUT |= BIT7;
    __delay_cycles(1000000);    //consente all'accelerometro di stabilizzarsi prima di campionare qualsiasi dato
}

/*AVVIO DI UNA CONVERSIONE*/
void start_conversion(){
    ADC10CTL0 |= ADC10SC;
    /*Genera il trigger per l'inizio della conversione portando a 1 il bit ADC10SC. Nella pulse sample mode SC viene riportato in automatico a 0 dall'ADC; non si ha la necessità
     * di riportarlo a 0 manualmente in maniera esplicita, lo si trova a 0 finita la conversione.*/
}

/*RITORNA LA CALIBRAZIONE DELL'ASSE X*/
long calibrazioneX (void){
    long calibX = 0;                    //valore della calibrazione
    long valueX = 0;                    //valore che tiene conto della somma dei vari valori convertiti per fare la media
    unsigned int calibCounterX = 0;     //contatore per la calibrazione

    ADC10CTL0 &= ~ADC10ENC;         //stop ADC
    ADC10MCTL0 = ADC10INCH_12;      //modifico il campo di interesse, ovvero il canale da convertire (12=X)
    ADC10CTL0 |= ADC10ENC;          //riabilito ADC
    while(calibCounterX < 10){
    start_conversion();             //parte la conversione
    while(ADC10CTL1 & ADC10BUSY);   //si aspetta che la conversione finisca
    valueX += ADC10MEM0;            //finita si incrementa Value con il valore ottenuto dalla conversione
    calibCounterX ++;
    }

    calibX = valueX/10;         //quando calibCounter arriva a 10 si fa la media

    return calibX;              //si ritorna la calibrazione
}

/*RITORNA LA CALIBRAZIONE DELL'ASSE Y*/
long calibrazioneY (void){
    long calibY = 0;                    //valore della calibrazione
    long valueY = 0;                    //valore che tiene conto della somma dei vari valori convertiti per fare la media
    unsigned int calibCounterY = 0;     //contatore per la calibrazione


    ADC10CTL0 &= ~ADC10ENC;         //stop ADC
    ADC10MCTL0 = ADC10INCH_13;      //modifico il campo di interesse, ovvero il canale da convertire (13=Y)
    ADC10CTL0 |= ADC10ENC;          //riabilito ADC
    while(calibCounterY < 10){
    start_conversion();            //parte la conversione
    while(ADC10CTL1 & ADC10BUSY);  //si aspetta che la conversione finisca
    valueY += ADC10MEM0;           //finita si incrementa Value con il valore ottenuto dalla conversione
    calibCounterY ++;
    }

    calibY = valueY/10;     //quando calibCounter arriva a 10 si fa la media

    return calibY;          //si ritorna la calibrazione
}

/*RITORNA LA CALIBRAZIONE DELL'ASSE Z*/
long calibrazioneZ (void){
    long calibZ = 0;                    //valore della calibrazione
    long valueZ = 0;                    //valore che tiene conto della somma dei vari valori convertiti per fare la media
    unsigned int calibCounterZ = 0;     //contatore per la calibrazione


    ADC10CTL0 &= ~ADC10ENC;         //stop ADC
    ADC10MCTL0 = ADC10INCH_14;      //modifico il campo di interesse, ovvero il canale da convertire (14=Z)
    ADC10CTL0 |= ADC10ENC;          //riabilito ADC
    while(calibCounterZ < 10){
    start_conversion();             //parte la conversione
    while(ADC10CTL1 & ADC10BUSY);   //si aspetta che la conversione finisca
    valueZ += ADC10MEM0;            //finita si incrementa Value con il valore ottenuto dalla conversione
    calibCounterZ ++;
    }

    calibZ = valueZ/10;             //quando calibCounter arriva a 10 si fa la media

    return calibZ;                  //si ritorna la calibrazione
}

/*FUNZIONE CHE RESTITUISCE LA LUNGHEZZA DEL NUMERO CHE SI METTE IN INGRESSO*/
int getLunghezza(int number){

    int i = 10;
    int len = 0;
    int temp = number;

    while(i>0){

        //si divide il numero per 10 finchè il suo valore è maggiore di 0 e si incrementa la lunghezza
        if((temp)>0){
            len ++ ;
        }else{
            i = 0;
        }
        temp = temp/i;
    }
    return len;     //ritorna #cifre

}

/*FUNZIONE CHE RESTITUISCE I VALORI PRIMA E DOPO LA VIRGOLA DEL VALORE CONVERTITO DALL'ADC IN mV*/
void adc_to_mV(volatile int arr[],volatile int valore){
    //long return_value[2];
    long v_uV = 0;      //tensione in uV
    long ppv = 0;       // parte prima della virgola
    long pdv = 0;       // parte dopo la virgola

    v_uV = (long)valore*(long)3525;

    ppv = v_uV/(long)1000;
    pdv = v_uV - ppv*(long)1000;

    arr[0] = ppv;
    arr[1] = pdv;

    /*trasforma il valore in uscita dall'ADC in mV, prendendo in ingresso il valore da trasformare e un array in cui mettere
     * in posizione 0 il valore convertito prima della virgola (parte intera) e in posizione 1 quello dopo la virgola (parte decimale) */

}

/*RITORNA IL VALORE DI CALORIE BRUCIATE IN BASE AI PARAMETRI D'INGRESSO (PESO E DISTANZA PERCORSA)*/
int calories (int peso, int metri){
    return ((peso*metri)/(2*1000));
}

/*RITORNA I METRI PERCORSI IN BASE AI PARAMETRI D'INGRESSO (ALTEZZA, SESSO E PASSSI)*/
int distance (int altezza,int passi,char sesso){
    int lunghezza_passo = 0;
    if(sesso=='M'){
        lunghezza_passo = (int)(0.415 * altezza);   //per sesso maschile

    }else{
        lunghezza_passo = (int)(0.413 * altezza);   //per sesso femminile

    }
    return ((lunghezza_passo * passi)/100);
}



