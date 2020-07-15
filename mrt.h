/*
 * mrt.h
 *
 * Created on: 1 lug 2020
 *     Author: betta_000
 */

#ifndef MRT_H_
#define MRT_H_

#include <msp430.h>

void init_delay();                  //funzione per inizializzare il timer utilizzato per il delay

void delay_clk(unsigned int clk);   //delay di 1us

void DelayMs(unsigned int ms);      //delay in ms

#define DelayUs(us) delay_clk(us)   //si definisce il delay in us uguale al delay_clk

#endif /* MRT_H_ */
