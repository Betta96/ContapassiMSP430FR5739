/*
 * mrt.c
 *
 * Created on: 1 lug 2020
 *     Author: betta_000
 */
#include "mrt.h"

//IMPOSTAZIONE TIMER
void init_delay(){
    // Inizializzazione timer per il delay
    TA0CTL |= TASSEL_2;                 //timer collegato a SMCLK
    TA0CTL |= ID_2;                     //divisore del timer a 4
    TA0EX0 |= TAIDEX_4;                  //ulteriore divisore del timer (5) per farlo andare a 1MHz dato che nelle impostazioni del clock fSMCLK = 20MHz
    TA0CTL |= MC_0;                     //stop mode
    TA0CTL &= ~TAIFG;                   //reset del flag
    TA0CCTL0 |= CCIE;                   // Attiva l'interrupt per il comparatore 0
}

//FUNZIONE PER UN DELAY DI 1us
void delay_clk(unsigned int clk){        // 1 clk = 1us

    TA0CCR0 = clk;              //conteggio massimo a cui si vuole che il timer arrivi
    TA0CTL |= MC_1;             //si imposta la up mode
    __bis_SR_register(CPUOFF);  //entra nella LPM0 mentre si aspetta che il timer finisca di contare
    __no_operation();
}

//FUNZIONE PER UN DELAY DI 1ms
void DelayMs(unsigned int ms){
    delay_clk(1000*ms);
}

/*
 *ROUTINE DI INTERRUPT DEL TIMER
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer0_a0_isr(){
    TA0CTL &= ~TAIFG;       //reset del flag
    TA0CTL &= ~MC_1;        //si ritorna alla stop mode
    __bic_SR_register_on_exit(CPUOFF);      //si esce dalla LPM dopo che il timer ha finito di contare
}

