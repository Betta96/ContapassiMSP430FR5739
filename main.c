#include <stdio.h>
#include <msp430.h> 
#include "HD44780_824.h"
#include "mrt.h"
#include "myfunctions.h"

/*
 * DICHIARAZIONI VARIABILI, COSTANTI E FUNZIONI
 */

volatile int subX = 0;       //valore convertito dall'adc calibrato (adc - calibrazione) inizializzato a 0
volatile int subY = 0;
volatile int subZ = 0;

volatile int adcX = 0;       //valore convertito dall'adc inizializzato a 0
volatile int adcY = 0;
volatile int adcZ = 0;


volatile unsigned int segnoX = 0;    //variabile che rappresenta il segno di "sub" può assumere valore 0 o 1 (0 -> sub positivo, 1 -> sub negativo)
volatile unsigned int segnoY = 0;
volatile unsigned int segnoZ = 0;

volatile unsigned int channelCounter = 13;   //variabile che rappresenta il canale da convertire

volatile unsigned int passi = 0;        // variabile che tiene il conto dei passi effettuati

volatile unsigned int posX = 0;       //variabile che indica la posizione di un elemento dell'array valoriX utilizzato quando si fa la media mobile
volatile unsigned int posY = 0;
volatile unsigned int posZ = 0;

volatile int valoriX[100] = {0};     //array che all'interno conserva i valori su cui fare la media mobile
volatile int valoriY[100] = {0};
volatile int valoriZ[100] = {0};

volatile int passo = 0;         //variabile che indica se un passo è stato effettuato o meno che può assumere i valori 0 o 1 (0 -> passo non fatto, 1-> passo fatto)

volatile int setup_altezza = 0;      //variabile che indica se è stata inserita o meno l'altezza dell'utente (0 -> non inserita, 1 -> inserita)
volatile int setup_peso = 0;         //variabile che indica se è stato inserito o meno il peso dell'utente (0 -> non inserito, 1 -> inserito)
volatile int setup_sesso = 0;        //variabile che indica se è stato inserito o meno il sesso dell'utente (0 -> non inserito, 1 -> inserito)

volatile unsigned int altezza = 160;         //si fa partire l'altezza dell'utente, che può essere incrementata o decrementata, da un valore medio 160
volatile unsigned int peso = 60;             //si fa partire il peso dell'utente, che può essere incrementato o decrementato, da un valore medio 60

volatile int distanza = 0;       //variabile che tiene conto della distanza percorsa in metri
volatile int calorie = 0;        //variabile che tiene conto delle kCal bruciate

char sesso = 'M';       //carattere che indica il sesso dell'utente, che può essere o F o M, si inizializza come M

const int soglia = 33;      //costante che indica la soglia che la media mobile dei valori dell'asse x dell'accelerometro deve superare per incrementare i passi

volatile int arrX[2];        //array passati alla funzione ADC_to_mV che conterranno la parte intera e decimale del valore convertito
volatile int arrY[2];
volatile int arrZ[2];

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    setup_clk();                //impostazione clock
    setup_adc();                //impostazione adc
    setup_S1();                 //impostazione tasto S1
    setup_S2();                 //impostazione tasto S2
    setup_external_button();    //impostazione tasto esterno

    enable_acc();           //abilitazione accelerometro

    calibrazioneY();    //si calibra l'asse y
    calibrazioneZ();    //si calibra l'asse z
    calibrazioneX();    //si calibra l'asse x

    __bis_SR_register(GIE);     //abilitazione degli interrupt generali
    __no_operation();


    init_delay();       // inizializzazione TA0 che si occupa dei delay
    InitLCD();          //inizializzazione LCD

    PutCommand(DISP_ON_CUR_ON_BLINK_ON);    //si accende il display LCD

    // finchè il sesso non è stato impostato, si chiede all'utente di inserirlo
    while(!setup_sesso){

        WriteString("Sesso : ", 0);
        WriteChar(sesso);
        WriteString("S1:+, S2:-, V:ok", 1);
    }

    PutCommand(DISPLAY_CLEAR);      //si ripulisce lo schermo e si riposiziona il cursore in posizione iniziale
    PutCommand(RETURN_HOME);
    DelayMs(1000);

    // finchè l'altezza non è stata impostata, si chiede all'utente di inserirla
    while(!setup_altezza){

        WriteString("Altezza (Cm) ", 0);
        Write_ndigitsval(altezza, getLunghezza(altezza));
        WriteString("S1:+, S2:-, V:ok", 1);
    }

    PutCommand(DISPLAY_CLEAR);      //si ripulisce lo schermo e si riposiziona il cursore in posizione iniziale
    PutCommand(RETURN_HOME);
    DelayMs(1000);

    // finchè il peso non è stato impostato, si chiede all'utente di inserirlo
    while(!setup_peso){

        WriteString("Peso (Kg) ", 0);
        Write_ndigitsval(peso, getLunghezza(peso));
        WriteString("S1:+, S2:-, V:ok", 1);
    }

    PutCommand(DISPLAY_CLEAR);      //si ripulisce lo schermo e si riposiziona il cursore in posizione iniziale
    PutCommand(RETURN_HOME);
    DelayMs(1000);

    WriteString("Inizia a", 0);
    WriteString("camminare!", 1);

    start_conversion();     //parte la prima conversione

    //dopo le impostazioni iniziali si entra nel loop infinito per la camminata
    while(1){

        adc_to_mV(arrX, subX);      //si trasformano in mV i valori convertiti dall'ADC
        adc_to_mV(arrY, subY);
        adc_to_mV(arrZ, subZ);

        // si riempe l'array usato per fare la media mobile. Una volta saturato si andrà a cambiare il valore più vecchio indicato da posX
        if(posX<100){
            valoriX[posX] = arrX[0];
            posX ++;
        }else{
            posX = 0;
        }
       /*
        *  per future implementazioni
        *
        *  if(posY<100){
            valoriY[posY] = arrY[0];
            posY ++;
        }else{
            posY = 0;
        }
        if(posZ<100){
            valoriZ[posZ] = arrZ[0];
            posZ ++;
        }else{
            posZ = 0;
        } */

        int sumX = 0;               //variabile usata per fare la somma dei valori su cui fare la media mobile
        int i;
        for(i = 0;i<100;i++){
            if(segnoX){            // se il segno è negativo sottraiamo il valore
                sumX -= valoriX[i];

            }else{                  // se il segno è positivo aggiungiamo il valore
                sumX += valoriX[i];
            }
        }

        int meanX = sumX/100;   // facciamo la media

        // se la media supera la soglia ricalcoliamo la media mobile finchè non diventa minore della soglia
        if(meanX >= soglia){
            while(meanX >= soglia){
                start_conversion();
                //media mobile
                if(posX<100){
                    valoriX[posX] = arrX[0];
                    posX ++;
                }else{
                    posX = 0;
                }
                sumX = 0;
                for(i = 0;i<100;i++){
                    if(segnoX){
                        sumX -= valoriX[i];

                    }else{
                        sumX += valoriX[i];
                    }
                }

                meanX = sumX/100;


            }
            passo = 1;  //quando si esce dall'attesa è stato effettuato un passo

        }

        //se il passo è stato effettuato, la variabile passo viene resettata e i passi vengono incrementati
        // e si scrive sull'LCD il valore totale dei passi, delle calorie bruciate e dei metri percorsi
        if(passo){
            passo = 0;
            passi ++;
            PutCommand(DISPLAY_CLEAR);
            PutCommand(RETURN_HOME);
            DelayMs(10);
            GoToLine(1);
            WriteString("Passi : ", 0);
            Write_ndigitsval(passi, getLunghezza(passi));
            GoToLine(2);
            distanza = distance(altezza, passi,sesso);
            calorie = calories(peso, distanza);
            WriteChar('k');
            WriteChar('C');
            WriteChar('a');
            WriteChar('l');
            WriteChar(':');
            Write_ndigitsval(calorie, 4);
            WriteChar('m');
            WriteChar(':');
            Write_ndigitsval(distanza,5);
        }

        __delay_cycles(100000);

        start_conversion();
    }

}

/*
 * ROUTINE DI INTERRUPT ADC
 */
#pragma vector=ADC10_VECTOR
__interrupt void conversion_end_handler(void){

    if (channelCounter == 12){
        ADC10CTL0 &= ~ADC10ENC;         //stop ADC
        ADC10MCTL0 = ADC10INCH_12;      //inserisco il prossimo canale da convertire (12=X)
        ADC10CTL0 |= ADC10ENC;          //riabilito ADC

        adcZ = ADC10MEM0;               //salvo i dati di interesse della conversiome appena terminata
        subZ = adcZ - calibrazioneZ();

        //se il valore calibrato è minore di 0, la variabile che tiene conto del segno viene messa ad 1 e il valore viene preso in modulo
        //altrimenti si lascia segno a 0 e si tiene il valore calibrato così com'è
        if (subZ<0){
            segnoZ = 1;
            subZ = -subZ;
        } else {
            segnoZ = 0;
        }

        channelCounter ++;
    }
    else if (channelCounter == 13){
        ADC10CTL0 &= ~ADC10ENC;         //stop ADC
        ADC10MCTL0 = ADC10INCH_13;      //inserisco il prossimo canale da convertire (13=Y)
        ADC10CTL0 |= ADC10ENC;          //riabilito ADC

        adcX = ADC10MEM0;               //salvo i dati di interesse della conversiome appena terminata
        subX= adcX - calibrazioneX();

        //se il valore calibrato è minore di 0, la variabile che tiene conto del segno viene messa ad 1 e il valore viene preso in modulo
        //altrimenti si lascia segno a 0 e si tiene il valore calibrato così com'è
        if (subX<0){
            segnoX = 1;
            subX = -subX;
        } else {
            segnoX = 0;
        }

        channelCounter ++;
    }

    else if (channelCounter == 14){
        ADC10CTL0 &= ~ADC10ENC;         //stop ADC
        ADC10MCTL0 = ADC10INCH_14;      //inserisco il prossimo canale da convertire (14=Z)
        ADC10CTL0 |= ADC10ENC;          //riabilito ADC

        adcY = ADC10MEM0;               //salvo i dati di interesse della conversiome appena terminata
        subY = adcY - calibrazioneY();

        //se il valore calibrato è minore di 0, la variabile che tiene conto del segno viene messa ad 1 e il valore viene preso in modulo
        //altrimenti si lascia segno a 0 e si tiene il valore calibrato così com'è
        if (subY<0){
            segnoY = 1;
            subY = -subY;
        } else {
            segnoY = 0;
        }

        channelCounter = 12;
    }

    return;
}

/*
 * ROUTINE DI INTERRUPT S1
 */
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void){


    if(setup_sesso){            //se il setup del sesso è stato effettuato

        if (P4IFG & BIT1){      //si verifica da quale tasto della porta 4 è arrivato l'interrupt. Se S2 è stato premuto si decrementano peso o altezza

            DisableSwitches();      //si disabilitano gli interrupt per il tasto per evitare di eseguire routine non richieste dovute al rimbazo del tasto

            __delay_cycles(1000000);         //tempo di attesa sempre per precauzione per il rimbalzo

            //se il setup dell'altezza non è stato ancora effettuato alla pressione del tasto si decrementa l'altezza, se è stato fatto si decrementa il peso
            if(!setup_altezza){
                altezza -=1;
                if (altezza > 215 || altezza < 130) {
                  altezza = 160;
                }
            } else if(!setup_peso){
                peso -=1;
                if (peso < 30 || peso > 200){
                    peso = 60;
                }
            }

        } else if (P4IFG & BIT0){       //Se S1 è stato premuto si incrementano peso o altezza

            DisableSwitches();          //si disabilitano gli interrupt per il tasto per evitare di eseguire routine non richieste dovute al rimbazo del tasto

            __delay_cycles(1000000);         //delay antirimbalzo

            //se sono gia stati effettuati i setup iniziali, la pressione del tasto S1 serve per calibrare
            if(setup_altezza && setup_peso){
                calibrazioneY();
                calibrazioneZ();
                calibrazioneX();
            }

            //se non è stato ancora effettuato setup dell'altezza alla pressione del tasto si incrementa l'altezza, se è stato fatto si incrementa il peso
            if(!setup_altezza){
                altezza ++;
                if (altezza > 215 || altezza < 130) {
                  altezza = 160;
                }
            } else if(!setup_peso){
                peso ++;
                if (peso < 30 || peso > 200){
                    peso = 60;
                }
            }
        }
    }
    else {       //se il setup del sesso non è ancora stato effettuato alla pressione di S1 o S2 si cambia il sesso visualzzato (F o M)

        DisableSwitches();      //si disabilitano gli interrupt per il tasto per evitare di eseguire routine non richieste dovute al rimbazo del tasto

        __delay_cycles(1000000);         //delay antirimbalzo

        if(sesso=='M'){
            sesso = 'F';
        }
        else {
            sesso = 'M';
        }
    }

    EnableSwitches(); //si riabilitano gli interupt del tasto

    return;
}

/*
 * ROUTINE DI INTERRUPT PER IL BOTTONE ESTERNO
 */
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void){

    DisableExternalSwitch();        //si disabilitano gli interrupt per il tasto per evitare di eseguire routine non richieste dovute al rimbazo del tasto

    __delay_cycles(1000000);        //tempo di attesa sempre per precauzione per il rimbalzo

    //il tasto esterno serve a confermare le impostazioni iniziali. se il setup del sesso non è ancora stato effettuato alla pressione del tasto si imposta il sesso, se è stato
    //fatto si imposta  il valore dell'altezza visualizzata, infine se è stata fatta anche l'altezza si conferma il peso e si termina il setup.
    if(!setup_sesso){
        setup_sesso = 1;
    }
    else if(!setup_altezza){
        setup_altezza = 1;
    }else if(!setup_peso){
        setup_peso =1;
    }

    EnableExternalSwitch();     //si riabilitano gli interupt del tasto

}
