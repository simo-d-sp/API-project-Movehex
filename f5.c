#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef struct airport Airport;
typedef struct hex Hex;

typedef struct hex{
    Airport * rotta;
    char modifica;
    int costo;
}Hex;

typedef struct cordinata{
    int x, y;
}Cordinata;

typedef struct supercordinata{
    int x, y, accumulo;
}Supercordinata;

typedef struct airport{
    Airport* next;
    Cordinata destinazione;
} Airport;



int nc, nr, r, z; //r e z sono rispettivamente il raggio e la grandezza del vettore di appoggio per la funzione changecost mentre nc numero colonne e nr numero righe (variabili globali)
Hex **M;
Supercordinata *s;
Supercordinata *tabella_h;
char flag0=0;
#define CAP 12000 //capacità dell'array di supporto s
#define CAP_H 12000 //capacità della hash table (deve essere un numero primo per la funzione di hash)

unsigned int hash_funzione(int x, int y) {
    return (x * 2654435761u + y * 2246822519u) % CAP_H;// funzione di hash trovata su reddit
}

void hash_in(int x, int y, int distanza) {
    unsigned int key = hash_funzione(x, y);
    unsigned int key_originale = key;

    while(tabella_h[key].x!=-1) {            //indirizzamento aperto
        if(tabella_h[key].x==-2)break;       //se trovo la lapide posso riutilizzare lo slot
        if(tabella_h[key].x == x && tabella_h[key].y == y) {

                tabella_h[key].accumulo = distanza; //condizione di aggiornamento presente dentro hash in.
                return;
        }
        key = (key + 1) % CAP_H;
        if(key == key_originale) {
            printf("tabella di hash piena\n");
            return;
        }
    }

    tabella_h[key].x = x;           // Inserisci in slot libero
    tabella_h[key].y = y;
    tabella_h[key].accumulo = distanza;
    return;
}

int hash_out(int x, int y) {                //mi restituisce il valore oppure -1 se non l'ho mai messo
    unsigned int key = hash_funzione(x, y);
    unsigned int key_originale = key;

    while(tabella_h[key].x!=-1 && tabella_h[key].x!=-2) {
        if(tabella_h[key].x == x && tabella_h[key].y == y) {
            return tabella_h[key].accumulo;
        }
        key = (key + 1) % CAP_H;
        if(key == key_originale) break; // Tutta la tabella scandita
    }
    return -1;
}

void hash_cancella(int x, int y){
    unsigned int key = hash_funzione(x, y);
    unsigned int key_originale = key;
    while(tabella_h[key].x!=-1) {
        if(tabella_h[key].x == x && tabella_h[key].y == y) {
            tabella_h[key].x=-2;  // metto la lapide
            return;
        }
        key = (key + 1) % CAP_H;
        if(key == key_originale) break; // Tutta la tabella scandita
    }
    //printf("errore, elemento non cancellato dall'hash\n");
    return;
}

void debug_matrix(int x, int y) {
    printf("Matrix state at (%d,%d): cost=%d, modifica=%c, air_routes=",
           x, y, M[y][x].costo, M[y][x].modifica);

    Airport *temp = M[y][x].rotta;
    while(temp != NULL) {
        printf("(%d,%d) ", temp->destinazione.x, temp->destinazione.y);
        temp = temp->next;
    }
    printf("\n");
}

void pulisci_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pulisci_matrice() {
    if (M != NULL) {

        for(int i = 0; i < nr; i++) {
            for(int j = 0; j < nc; j++) {
                Airport *current = M[i][j].rotta;
                while (current != NULL) {
                    Airport *temp = current;
                    current = current->next;
                    free(temp);
                }
                M[i][j].rotta = NULL;
            }
        }


        for(int i = 0; i < nr; i++) {
            free(M[i]);
        }
        free(M);
        M = NULL;
    }
}


int max(int a,int b){
    if(a>b) return a;
    else return b;
}




void heapify_in(Supercordinata *s, int i){   // meglio di prima! Non sto ogni volta ad inserire elementi e a chiamare min_heapifyogni volta!
    while (i > 0) {
        int padre = (i - 1) / 2;
        if (s[i].accumulo < s[padre].accumulo) {
            Supercordinata temp = s[i];
            s[i] = s[padre];
            s[padre] = temp;
            i = padre;
        } else break;
    };
    return;
}
void heapify_out(Supercordinata *s, int j){
    int i = 0;
    while (1) {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        int min = i;

        if (l < j && s[l].accumulo < s[min].accumulo)
            min = l;
        if (r < j && s[r].accumulo < s[min].accumulo)
            min = r;

        if (min != i) {
            Supercordinata temp = s[i];
            s[i] = s[min];
            s[min] = temp;
            i = min;
        } else break;
    }
    return;
}



void init () {

    pulisci_matrice();
    if(scanf("%d %d", &nc, &nr) != 2) {
        printf("KO\n");
        return;
    }
    if(flag0==0){
        s=malloc(CAP*sizeof(Supercordinata));     //solo una volta all'inizio e free alla fine per non riallocarlo ogni volta
        flag0=1;
        tabella_h=malloc(CAP_H*sizeof(Supercordinata));
    }
    pulisci_buffer();


    M = malloc(nr*sizeof(Hex* ));

    for(int i=0; i<nr; i++){
        M[i]= malloc(nc*sizeof(Hex));
    }

    for(int j=0; j<nc; j++){
        for(int i=0; i<nr; i++){
          //  M[0][j].costo=1;
          //  M[0][j].modifica='0';
            M[i][j].modifica =0;
            M[i][j].costo =1;
            M[i][j].rotta = NULL;
        }
    }


   printf("OK\n");


    return;
}





void change_cost() {                      //qui uso supercordinata come array circolare
    int x0, y0, v, xe, ye;
    int Yp[6]={0,-1,-1,0,1,1};
    int Xp[6]={1,0,-1,-1,-1,0};
    int Yd[6]={1,0,-1,-1,0,1};
    int Xd[6]={1,1,1,0,-1,0};
    if(scanf("%d%d%d%d", &x0, &y0, &v, &r)!=4){
        printf("-1\n"); return;
    }
    pulisci_buffer();

    if(x0<0 || x0>=nc || y0<0 || y0>=nr || v<-10 || v>10 || r==0){
        printf("KO\n"); return;
    }


    s[0].x=x0; s[0].y=y0;
    s[0].accumulo=0;
    M[y0][x0].modifica=1;


    int testa=0, coda = 1; //  Gestione coda con testa/coda
    int pcosto;
    int dist_corrente;
    char flag_c=0;
    while(coda >= 0 && coda!= testa ) {
        if(testa==CAP)testa=0;
        xe = s[testa].x;
        ye = s[testa].y;
        dist_corrente = s[testa].accumulo;
        testa++;

        pcosto = (int)floor((v * ((float)(r - dist_corrente) / r)));
        if (ye >= 0 && ye < nr && xe >= 0 && xe < nc) {


            M[ye][xe].costo = max(0, M[ye][xe].costo + pcosto);
            //debug_matrix(xe, ye);

            for(int i = 0; i < 6; i++) {
                if(coda==CAP) coda=0; //metto questo controllo qui perchè è qui che coda viene aggiornata
                if(flag_c==1 && coda==testa) break;
                int check_x, check_y;
                if(ye % 2 == 0) {
                    check_x = xe + Xp[i];
                    check_y = ye + Yp[i];
                } else {
                    check_x = xe + Xd[i];
                    check_y = ye + Yd[i];
                }

                if(check_x >= 0 && check_x < nc && check_y >= 0 && check_y < nr) {
                    if(coda==CAP) coda=0; //metto questo controllo qui perchè è qui che coda viene aggiornata
                    if(flag_c==1 && coda==testa) break;
                    flag_c=1;
                    if(M[check_y][check_x].modifica == 0 && dist_corrente + 1 < r) {
                        M[check_y][check_x].modifica =1;
                        s[coda].x = check_x;        // Accoda nuovo elemento
                        s[coda].y = check_y;
                        s[coda].accumulo = dist_corrente + 1;
                        coda++;
                    }
                }
            }
        }
    }

    for(int i=0;i<nr;i++) {
        for(int j=0;j<nc;j++) {
            M[i][j].modifica=0;
        }
    }
    printf("OK\n");
}

void toggle_air_route(){
    int x1, y1, x2, y2;
    if(scanf("%d %d %d %d", &x1, &y1, &x2, &y2) != 4) {
        printf("KO\n");
        return;
    }
    pulisci_buffer();
   if(x1<0 || x1>=nc || x2<0 || x2>=nc || y1<0 || y1>=nr || y2<0 || y2>=nr){
       printf("KO\n");
       return;
   }
   Airport *temp = M[y1][x1].rotta;
   Airport *prev = NULL;




    if(M[y1][x1].rotta==NULL){
        Airport *newconnessione=malloc(sizeof(Airport));
        newconnessione->destinazione.x=x2;
        newconnessione->destinazione.y=y2;
        newconnessione->next=NULL;
        M[y1][x1].rotta=newconnessione;
        newconnessione=NULL;
        printf("OK\n");
        //debug_matrix(x1, y1);
        return;
    }
    else{
        int i=0;
        while (temp!=NULL && i<5){

            if(temp->destinazione.x==x2 && temp->destinazione.y==y2){
                if(i==0){
                    M[y1][x1].rotta=temp->next;
                   free(temp);
                   prev=NULL;
                   printf("OK\n");
                    return;
                }

                else {
                    prev->next=temp->next;
                    temp->next=NULL;
                    free(temp);
                    printf("OK\n");
                    //debug_matrix(x1, y1);
                    return;
                }
            }

            if(temp->next==NULL){
                Airport *newconnessione= malloc(sizeof(Airport));
                newconnessione->destinazione.x=x2;
                newconnessione->destinazione.y=y2;
                temp->next=newconnessione;
                newconnessione->next=NULL;
                printf("OK\n");
                //debug_matrix(x1, y1);
                return;
            }

            prev=temp;
            temp=temp->next;
            i++;
        }
        // printf("La rotta da (%d,%d) a (%d,%d) non è stata trovata tra le prime %d\n", x1, y1, x2, y2, i);
    }
    printf("KO\n");

    return;
}





void travel_cost() {
    int xp, yp, xd, yd;
    int Yp[6]={0,-1,-1,0,1,1};
    int Xp[6]={1,0,-1,-1,-1,0};
    int Yd[6]={1,0,-1,-1,0,1};
    int Xd[6]={1,1,1,0,-1,0};
    int acc; //costo accumulato
    char flag_t=0;
    if(scanf("%d%d%d%d", &xp,&yp,&xd,&yd)!=4){
        printf("KO\n");
        return;

    }
    pulisci_buffer();

    if(xp<0 || xp>=nc || xd<0 || xd>=nc || yp<0 || yp>=nr || yd<0 || yd>=nr){
        printf("-1\n");
        return;
    }
    if(xp==xd && yp==yd){
        printf("0\n");
        return;
    }
    for(int i=0; i<CAP_H; i++){
        tabella_h[i].x=-1;
    }

    int j=1;
    s[0].x=xp; s[0].y=yp;
    s[0].accumulo=0;
    M[yp][xp].modifica=1;
    while(j>0){
        int xe=s[0].x, ye=s[0].y, h_value=hash_out(xe,ye);
        if(s[0].accumulo> h_value && h_value!=-1){ //istruzione che mi permette di verificare che sia il cammino minimo
            acc=s[0].accumulo+M[ye][xe].costo;
            s[0]=s[j-1];
            j--;
            if(flag_t==1) heapify_out(s,j); // heap solo sui primi j elementi
            continue;
        }
        if(xe==xd && ye==yd){
            printf("%d\n",s[0].accumulo);
            for(int i=0;i<nr;i++) {
                for(int j=0;j<nc;j++) {
                    M[i][j].modifica=0;
                }
            }
            return;
        }
        if(M[ye][xe].costo==0){
            acc=s[0].accumulo+M[ye][xe].costo;
            s[0]=s[j-1];
            j--;
            M[ye][xe].modifica=1;
            if(flag_t==1){
                heapify_out(s, j);
                hash_cancella(xe,ye);
            }
            continue;
        }
        acc=s[0].accumulo+M[ye][xe].costo;
        s[0]=s[j-1];
        j--;
        M[ye][xe].modifica=1;
        if(flag_t==1){
            heapify_out(s, j);
            hash_cancella(xe,ye);
        }
        flag_t=1;


        for(int i=0;i<6;i++){
            int check_x;
            int check_y;
            if(ye % 2 == 0) {
                check_x = xe + Xp[i];
                check_y = ye + Yp[i];
            } else {
                check_x = xe + Xd[i];
                check_y = ye + Yd[i];
            }
            if(check_x<0 || check_x>=nc || check_y<0 || check_y>=nr) continue;
            if(M[check_y][check_x].modifica) continue;
            h_value=hash_out(check_x,check_y);
            if(h_value==-1 || h_value>acc){
                hash_in(check_x,check_y, acc);
                s[j].x=check_x;
                s[j].y=check_y;
                s[j].accumulo=acc;
                heapify_in(s, j);
                j++;
            }

        }

        Airport *temp = M[ye][xe].rotta; //espando rotte aeree
        while(temp){
            int xa=temp->destinazione.x, ya=temp->destinazione.y;
            if(!M[ya][xa].modifica && M[ya][xa].costo!=0){
                h_value=hash_out(xa, ya);
                if(h_value==-1 || h_value>acc){
                    hash_in(xa,ya, acc);
                    s[j].x=xa; s[j].y=ya;
                    s[j].accumulo=acc;
                    heapify_in(s, j);
                    j++;
                }
            }
            temp=temp->next;
        }
    }

    for(int i=0;i<nr;i++) {
        for(int j=0;j<nc;j++) {
            M[i][j].modifica=0;
        }
    }
    printf("-1\n");
    return;
}





void menu(){
    char funzione[17];

    while(1){
        int d = scanf("%16s", funzione);
        if(d == EOF) {
            break;
        }
        if(d != 1) {

            continue;
        }

        if(strcmp(funzione,"init")==0) init();
        else if(strcmp(funzione,"travel_cost")==0) travel_cost();
        else if(strcmp(funzione,"change_cost")==0) change_cost();
        else if(strcmp(funzione,"toggle_air_route")==0) toggle_air_route();
        else if(strcmp(funzione,"debug_matrix")==0){
            int a=0, b=0;
            if(scanf("%d %d", &a, &b)!=2){
               printf("KO\n");
               return;
            }
            debug_matrix(a, b);

        }
    }
    return;
}



int main (){

menu();
pulisci_matrice();
free(s);
free(tabella_h);

return 0;
}


