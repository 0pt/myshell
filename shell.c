#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

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
	printf("# ifn = \"%s\"\n# ofn = \"%s\"\n",cmdar.ifn,cmdar.ofn);
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

// pick redirect area
int pickrdirIN(char *dst, char *src){
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

int pickrdirOUT(char *dst, char *src){
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

cmdrec analyze_buf(cmdrec cmdar, char *cmd){
	printf("# analyze_odr()\n");
	int cmdlen = strlen(cmd);

	cmdar.ifn = (char *)malloc(sizeof(char)*cmdlen);
	int m_in = pickrdirIN(cmdar.ifn, cmd);
	printf("# cmdar.ifn = \"%s\"\n# cmd = \"%s\"\n",cmdar.ifn, cmd);
	if(cmdar.ifn && cmdar.ifn[0]){
		cmdar.ifd = open(cmdar.ifn, O_WRONLY|O_CREAT|O_TRUNC, 0644);
 	}
	printf("# cmdar.ifd = %d\n",cmdar.ifd);

	cmdar.ofn = (char *)malloc(sizeof(char)*cmdlen);
	int out_m = pickrdirOUT(cmdar.ofn, cmd);
	printf("# cmdar.ofn = \"%s\"\n# cmd = \"%s\"\n",cmdar.ofn, cmd);
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
		analyze_buf(cmdar, buf);
		pid = fork();
		
		// in child proc
		if(pid == 0){
			execlp(buf, buf, (char *)NULL);
			perror("execlp");
			exit(1);
		}
		while(wait(&status) != pid){
			;
		}
	}
}
