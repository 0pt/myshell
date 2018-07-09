/* i442 kadai2
 * 拡張事項：
 * 複数の区切り文字を扱える同様な関数を作る done
 * 区切り文字を正規表現で指定する同様な関数を作る
 * 格納領域も同時に確保する同様な関数を作る
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>
#include "split.h"

void show(char **dar){
	for(int i=0; dar[i]!=0; i++){
		printf("%s\n",dar[i]);
	}
}

int split(int n, int w, char **dar, char *src, int del){
	for(int i=0; i<n; i++){
		for(int j=0; j<w+1; j++){
			// if n is small
			if((*src == del || j == w) && i == n-1){
				dar[i][j] = '\0';
				dar[i+1] = 0;
				return 0;
			}

			if(*src == del || j == w){
				dar[i][j] = '\0';
				while(*src != del && *src != '\0'){
					src++;
				}
				if(*src == '\0'){
					dar[i+1] = 0;
					return 0;
				}
				src++;

				while(*src == del){
					src++;
				}
				break;
			}else if(*src == '\0'){
				dar[i][j] = '\0';
				dar[i+1] = 0;
				return 0;
			}else{
				dar[i][j] = *src;
				src++;
			}
		}
	}
	
	return -1;
}

int split_multi(int n, int w, char **dar, char *src, char *del){
	bool flag = false;
	for(int i=0; i<n; i++){
		for(int j=0; j<w+1; j++){
			// printf("%d %d",i,j);
			flag = false;
			for(int k=0; k<strlen(del); k++){
				if(del[k] == *src){
					flag = true;
				}
			}

			if((flag || j == w) && i == n-1){
				dar[i][j] = '\0';
				dar[i+1] = 0;
				return 0;
			}

			if(flag || j == w){
				dar[i][j] = '\0';
				while(true){
					flag = false;
					for(int k=0; k<strlen(del); k++){
						if(del[k] == *src || *src == '\0'){
							flag = true;
							break;
						}
					}
					if(flag) break;
					src++;
				}

				src++;
				while(true){
					flag = false;
					for(int k=0; k<strlen(del); k++){
						if(del[k] == *src){
							flag = true;
							break;
						}
					}
					if(flag) src++;
					else break;
				}
				break;
			}else if(*src == '\0'){
				dar[i+1] = 0;
				return 0;
			}else{
				dar[i][j] = *src;
				src++;
			}
		}
	}
	dar[n] = 0;
	return -1;
}

char **allocsplit(char *src, int del){
	int slen = strlen(src);
	char *tmp = (char *)malloc(sizeof(char)*(slen+1));
	int cnt = 0;
	for(int i=0; i<slen; i++){
		if(*(src+i) == del) cnt++;
	}
	char **dar = (char **)malloc(sizeof(char *)*(cnt+1));
	bool flag = true;
	while(*src != '\0'){
		if(flag){
			*dar = tmp;
			dar++;
			flag = false;
		}
		if(*src == del){
			*tmp = '\0';
			tmp++;
			flag = true;
		}else{
			*tmp = *src;
			tmp++;
		}
		src++;
	}
	*tmp = '\0';
	*dar = 0;
	return dar-cnt-1;
}


