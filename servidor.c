#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMFILA      5
#define MAXHOSTNAME 30
#define NMSG 1000000

typedef struct cliente{
  int addr;
  int *num;
  int tam;
  int fdo;
} cliente;

void inicia_clt(cliente *clt, int addr); //inicia estrutura cliente

void inicia_clt(cliente *clt, int addr){
  clt->addr = addr;
  clt->num = calloc(NMSG, sizeof(int));
  clt->tam = 0;
  clt->fdo = 0;
}

int main ( int argc, char *argv[] ) {
  cliente clt[10];
  int s, t, ncl = 1, mcnt;
  unsigned int i;
  struct sockaddr_in sa, isa;  /* sa: servidor, isa: cliente */
  struct hostent *hp;
  char localhost [MAXHOSTNAME];

  if (argc != 2) {
    puts("Uso correto: servidor <porta>");
    exit(1);
  }

  gethostname (localhost, MAXHOSTNAME);

  if ((hp = gethostbyname( localhost)) == NULL){
    puts ("Nao consegui meu proprio IP");
    exit (1);
  }	
	
  sa.sin_port = htons(atoi(argv[1]));

  bcopy ((char *) hp->h_addr, (char *) &sa.sin_addr, hp->h_length);

  sa.sin_family = hp->h_addrtype;		

  printf("nome %s\n", localhost);
  
  if ((s = socket(hp->h_addrtype,SOCK_DGRAM,0)) < 0){
    puts ( "Nao consegui abrir o socket" );
    exit ( 1 );
  }	

  if (bind(s, (struct sockaddr *) &sa,sizeof(sa)) < 0){
    puts ( "Nao consegui fazer o bind" );
    exit ( 1 );
  }		

  int nt1, nt2 = 0;
  int ftp = 0;
  float ftf= 0;

  puts("n testes:");
  scanf("%d", &nt1);
  puts("n clientes:");
  scanf("%d", &ncl);

  char strh[50];
  sprintf(strh, "%d", ncl);
  strcat(strh, localhost);
  strcat(strh, "-log-s.txt");
  FILE *log = fopen(strh, "w");//cria arquivo log
  if (!log){
    puts("Erro ao criar arquivo.");
    exit(1);
  }
  strcat(strh, ".dat");
  FILE *fp = fopen(strh, "w");//arquivo das porcentagens
  if (!fp){
    puts("Erro ao criar arquivo.");
    exit(1);
  }
  
  fprintf(fp, "#numero de testes: %d\n", nt1);
  fprintf(log, "Nome do host: %s\nNumero de testes: %d\n\n", localhost, nt1);

  //comeca a receber
  while (nt2 < nt1){
    fprintf(log, "\nComeco do %d teste.\n", nt2+1);
    i = sizeof(isa);
    puts("Primeira msg.");

    //recebe msg
    recvfrom(s, &mcnt, sizeof(int), 0, (struct sockaddr *) &isa, &i);
    inicia_clt(&clt[0], ntohl(isa.sin_addr.s_addr));
    clt[0].num[0] = mcnt;
    clt[0].tam++;
    puts("criou cliente no array.");
    ncl = 1;

    //recebeu primeira msg, guarda cliente.
    fprintf(log, "\nNovo cliente: N.%d. ID: %u\n\n", ncl, ntohl(isa.sin_addr.s_addr));
    int fim = 0;
  
    while (!fim) {//enquanto nao terminar de receber de todos clientes.
      i = sizeof(isa);
      //recebe msg
      recvfrom(s, &mcnt, sizeof(int), 0, (struct sockaddr *) &isa, &i);
      if (mcnt == -1){//fim de todos clientes
	fim = 1;
      } else{
	int end_cli = ntohl(isa.sin_addr.s_addr);
	int achou = 0, j;
	for (j=0; j<ncl && !achou; ++j){
	  if (clt[j].addr == end_cli){ // cliente existe 
	    clt[j].num[clt[j].tam] = mcnt;
	    clt[j].tam++;

	    if ((clt[j].tam%(NMSG/10)) == 0)
	      //recebeu msg de cliente existente
	      fprintf(log, "Recebeu msg numero %d do cliente ID %u\n", mcnt, end_cli);
	    achou = 1;
	  }
	}
	if (!achou){ //!achou, cliente novo
	  inicia_clt(&clt[ncl], end_cli);
	  clt[ncl].num[0] = mcnt;
	  clt[ncl].tam++;
	  ++ncl;

	  printf("novo cliente.\n");
	  //registra cliente.
	  fprintf(log, "\nNovo cliente: N.%d. ID: %u\n\n", ncl, ntohl(isa.sin_addr.s_addr));
	}
      }
    }

    fprintf(log, "\nFim dos recebimentos.\nTotal de clientes: %d\nVerificando ordem e perdas de mensagens...\n", ncl);

    puts("ordenando e vendo perdas...");

    int k, l, aux[NMSG];

    //verifica fora de ordem
    for (k=0;k<ncl;++k){
      for (l=0;l<clt[k].tam-1;++l)
	if (clt[k].num[l+1]<clt[k].num[l])
	  clt[k].fdo++;
    }

    //faz media das perdas e fora de ordem
    int tp = 0, tf= 0;
    for (k=0; k<ncl; ++k){
      tp += NMSG-clt[k].tam;
      tf += clt[k].fdo;
    }

    ftp += tp/ncl;
    ftf += tf/ncl;

    for(k=0; k<ncl;++k)
      free(clt[k].num);
    nt2++;
    fprintf(log, "Recebeu %d mensagens, perdeu %d e %d fora de ordem.\n\nFim do %d teste.\n\n\n", (NMSG*ncl)-tp, tp, tf, nt2);
  }

  ftp /= nt1;
  ftf /= nt1;
  printf("%f\n", ftf);
  fprintf(log, "Fim dos testes.\nMedia: Recebeu %d mensagens, perdeu %d e %2.3f fora de ordem.\n", NMSG-ftp, ftp, ftf);
  fprintf(fp, "%d\t%f\t%f\n", ncl, ((float)ftp*100)/NMSG, (ftf*100)/NMSG);
  
  fclose(fp);
  fclose(log);
}
