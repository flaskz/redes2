#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define NMSG 1000000

main(int argc, char *argv[]) {
  int sockdescr;
  int numbytesrecv;
  struct sockaddr_in sa;
  struct hostent *hp;
  char localhost[30];
  char *host;

  unsigned int i;

  gethostname(localhost, 30);

  char strh[50];
  strcpy(strh, localhost);
  strcat(strh, "-");
  strcat(strh, argv[1]);
  strcat(strh, "-log-c.txt");
  
  FILE *fp = fopen(strh, "w");//cria arquivo log
  if (!fp){
    puts("Erro ao criar arquivo.");
    exit(1);
  }

  fprintf(fp, "Cliente: %s.\n\nEnviando %d mensagens para %s.\n", localhost, NMSG, argv[1]);
  
  if(argc != 3) {
    puts("Uso correto: <cliente> <nome-servidor> <porta>");
    exit(1);
  }

  host = argv[1];

  if((hp = gethostbyname(host)) == NULL){
    puts("Nao consegui obter endereco IP do servidor.");
    exit(1);
  }
  
  bcopy((char *)hp->h_addr, (char *)&sa.sin_addr, hp->h_length);
  sa.sin_family = hp->h_addrtype;

  sa.sin_port = htons(atoi(argv[2]));

  if((sockdescr=socket(hp->h_addrtype, SOCK_DGRAM, 0)) < 0) {
    puts("Nao consegui abrir o socket.");
    exit(1);
  }

  int nt1, nt2 = 0;
  puts("n testes.");
  scanf("%d", &nt1);
  fprintf(fp, "Numero de testes %d.\n", nt1);
  
  //comeca a enviar
  while (nt2 < nt1){
    int cnt = 0;

    fprintf(fp, "Comeco do %d teste.\n", nt2+1);
    
    while (cnt < NMSG){
      if(sendto(sockdescr, &cnt, sizeof(int), 0, (struct sockaddr *) &sa, sizeof sa) != sizeof(int)){
	puts("Nao consegui mandar os dados"); 
	exit(1);//tirar
      }
      if ((cnt%(NMSG/10)) == 0)
      //enviou msg cnt.
	fprintf(fp, "Enviou msg %d para %s.\n", cnt, argv[1]);
      ++cnt;
    }
    //ultima msg
    fprintf(fp, "Enviou msg %d para %s.\n", cnt-1, argv[1]);
    nt2++;
    printf("terminou da %dx\n", nt2);
    //printf("%d\n", cnt-1);
    int q;
    puts("fim? s/n");
    scanf("%d", &q);
    if(q==-1)
      if(sendto(sockdescr, &q, sizeof(int), 0, (struct sockaddr *) &sa, sizeof sa) != sizeof(int)){
	puts("Nao consegui mandar os dados"); 
	exit(1);
      }
    fprintf(fp, "Fim do %d teste.\n\n", nt2);
  }

  fprintf(fp, "Fim de todos os testes. Enviou %d mensagens ao servidor %s.", nt1*NMSG, argv[1]);
  
  fclose(fp);
  close(sockdescr);
  exit(0);
}
