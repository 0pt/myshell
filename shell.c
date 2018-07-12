#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "split.h"

#define RDIR_IN		(10)
#define RDIR_HERE	(11)
#define RDIR_OUT	(20)
#define RDIR_APPEND	(21)

#define DUP2(xx, yy) if((xx)>=0) { dup2((xx),(yy)); }
#define CLOSE(xx) if((xx)>=0) { close(xx); }

// ifd, ofd should be initialized -1
typedef struct{
	pid_t pid;
	char *cmdarg[10]; /* split result */
	char *ifn; int ifd;
	char *ofn; int ofd;
} cmdrec;

int div_cnt(char *s, char del){
	int cnt = 0;
	for(int i=0; i<strlen(s); i++){
		if(*s == del){
			cnt++;
		}
		s++;
		while(*s == del){
			s++;
		}
	}
	return cnt;
}

void show_cmdrec(cmdrec cmdar){
	printf("# show_cmdrec()\n");
	printf("# pid = %d, ifd = %d, ofd = %d\n",cmdar.pid,cmdar.ifd,cmdar.ofd);
	printf("# ifn = %s\n# ofn = %s\n",cmdar.ifn,cmdar.ofn);
	for(int i=0; i<10; i++){
		printf("# %d : %s\n",i ,cmdar.cmdarg[i]);
	}
}

void init_dar(char **dar, int n){
	dar = (char **)malloc(sizeof(char *)*n);
	for(int i=0; i<n; i++){
		*(dar+i) = (char *)malloc(sizeof(char)*BUFSIZ);
	}
}

int check_file(char c){
	if(c >= 'a' && c <= 'z'){
		return 1;
	}else if(c >= 'A' && c <= 'Z'){
		return 1;
	}else if(c >= '0' && c<= '9'){
		return 1;
	}else if(c == '.' || c == '-'){
		return 1;
	}
	return 0;
}

// slide hint03 p.7 ; pick redirect area
int pickrdirIn(char *dst, char *src){
	printf("# pickrdirIN()\n");
	int m=0; char *p = src, *q = dst, *c;
	// printf("%c\n",*p);
	while(*p){
		if(*p == '<'){
			c = p;
			m = RDIR_IN;
			p++;
			if(*p == '<'){
				m = RDIR_HERE;
				p++;
			}
			while(*p && (*p == ' ' || *p == '\t')){
				p++;
			}
			while(*p && check_file(*p)){
				*q++ = *p++;
			}
			while(c < p) *c++ = ' ';
			*q = '\0';
			break;
		}
		p++;
	}
	return m;
}

int pickrdirOut(char *dst, char *src){
	printf("# pickrdirOUT()\n");
	char *p, *q, *c; int m=0;
	p = src;
	q = dst;
	*q = '\0';
	while(*p){
		if(*p == '>'){
			c = p;
			m = RDIR_OUT;
			p++;
			if(*p == '>'){
				m = RDIR_APPEND;
				p++;
			}
			while(*p && (*p == ' ' || *p == '\t')){
				p++;
			}
			while(*p && check_file(*p)){
				*q++ = *p++;
			}
			while(c < p) *c++ = ' ';
			*q = '\0';
			break;
		}
		p++;
	}
	return m;
}

cmdrec init_cmdrec(cmdrec cmd){
	cmd.ifd = -1;
	cmd.ofd = -1;
	return cmd;
}

cmdrec analyze_buf(cmdrec cmdar, char *cmd, int *m_in, int *m_out){
	printf("# analyze_odr()\n");
	int cmdlen = strlen(cmd);
	// printf("%s\n", cmd);
	cmdar.ifn = (char *)malloc(sizeof(char)*cmdlen);
	*m_in = pickrdirIn(cmdar.ifn, cmd);
	printf("# cmdar.ifn = \"%s\", cmd = \"%s\"\n",cmdar.ifn, cmd);
	if(cmdar.ifn && cmdar.ifn[0]){
		cmdar.ifd = open(cmdar.ifn, O_WRONLY|O_CREAT|O_TRUNC, 0644);
 	}
	printf("# cmdar.ifd = %d\n",cmdar.ifd);
	
	cmdar.ofn = (char *)malloc(sizeof(char)*cmdlen);
	*m_out = pickrdirOut(cmdar.ofn, cmd);
	printf("# cmdar.ofn = \"%s\", cmd = \"%s\"\n",cmdar.ofn, cmd);
	if(cmdar.ofn && cmdar.ofn[0]){
		cmdar.ofd = open(cmdar.ofn, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	}
	printf("# cmdar.ofd = %d\n",cmdar.ofd);
	return cmdar;
}

int main(){
	pid_t pid;
	int status;
	char buf[BUFSIZ];
	while(true){
		printf("$ ");
		if(fgets(buf, sizeof(buf), stdin) == NULL) break;
		buf[strlen(buf)-1] = '\0';
		cmdrec cmdar;
		cmdar = init_cmdrec(cmdar);
		int m_in = 0, m_out = 0;
		cmdar = analyze_buf(cmdar, buf, &m_in, &m_out);
		printf("# m_in, m_out = %d, %d\n",m_in, m_out);
		pid = fork();
		cmdar.pid = pid;
		// pid == 0 is child proc
		if(cmdar.pid == 0){
			char **tmp = (char **)malloc(sizeof(char)*11);
			for(int i=0; i<10; i++){
				*(tmp+i) = (char *)malloc(sizeof(char)*BUFSIZ);
			}
			split(10, BUFSIZ, tmp, buf, ' ');
			show(tmp);
			for(int k=0; k<10; k++){
				if(!*(tmp+k)){
					break;
				}
				cmdar.cmdarg[k] = *(tmp+k);
				
				// because my split() cause bag, it just replace NULL
				if(cmdar.cmdarg[k][0] == '\0') cmdar.cmdarg[k] = NULL;
			}
			show_cmdrec(cmdar);
			// execlp(buf, buf, (char *)NULL);
			execvp(cmdar.cmdarg[0], cmdar.cmdarg);
			perror("execvp");
			exit(1);
		}
		while(wait(&status) != pid){
			;
		}
	}
}
