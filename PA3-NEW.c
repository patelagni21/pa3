#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Line{
    int valid;
    char *tag;
    char *block;
    int set;
    int recent;
    int tag_length;
}Line;

int total_hits=0, total_misses=0, total_reads=0, total_writes=0;
int lines_cnt=0, assoc=0;
int block_sub=0, set_sub=0;
Line* cache;

Line *cache_create(){
    Line *cache = malloc(sizeof(Line) * lines_cnt);
    for(int i = 0; i < lines_cnt; i++){
        cache[i].valid = 0;
        cache[i].tag = (char *)malloc(sizeof(char) * 81);
        strcpy(cache[i].tag, "-");
        cache[i].set = i / assoc;
        cache[i].recent = 0;
        cache[i].tag_length = 1;
    }
    return cache;
}

int calculate_tag_length(char addr[]){
    return strlen(addr) - set_sub - block_sub;
}

void convert_binary_address(char addr[], char binary[]){
    //char *binary = (char *)malloc(sizeof(char) * 81);
    //char binary[50];
    for(int i=2; i < strlen(addr); i++){
        switch(addr[i]){
            case '0':
            	//printf("sai-befor: binary:%s \n", binary);
                strcat(binary, "0000");
                break;
            case '1':
                strcat(binary, "0001");
                break;
            case '2':
                strcat(binary, "0010");
                break;
            case '3':
                strcat(binary, "0011");
                break;
            case '4':
                strcat(binary, "0100");
                break;
            case '5':
                strcat(binary, "0101");
                break;
            case '6':
                strcat(binary, "0110");
                break;
            case '7':
                strcat(binary, "0111");
                break;
            case '8':
                strcat(binary, "1000");
                break;
            case '9':
                strcat(binary, "1001");
                break;
            case 'a':
                strcat(binary, "1010");
                break;
            case 'b':
                strcat(binary, "1011");
                break;
            case 'c':
                strcat(binary, "1100");
                break;
            case 'd':
                strcat(binary, "1101");
                break;
            case 'e':
                strcat(binary, "1110");
                break;
            case 'f':
                strcat(binary, "1111");
                break;
        }
		//printf("sai-befor: binary:%s \n", binary);
    }
    //return binary;
}

int fetch_index(char address[], int tag_length){
    int index = 0;
    int exp = 1;
    for(int i = strlen(address) - 1 - block_sub; i >= tag_length; i--){
        if(address[i] == '1'){
            index += exp;
        }
        exp = exp << 1;
    }
    return index;
}

void recents_update(int new_index, int index){
    for(int i = index * assoc; i < index; i++){
        cache[i].recent++;
    }
    cache[new_index].recent = 0;
}

void cache_write(char addr[])
{
    int max = 0;
    int index = fetch_index(addr, calculate_tag_length(addr)) * assoc;
    int max_index = index;
    total_writes++;
    for(int i = index; i < index + assoc; i++){
        if(strncmp(cache[i].tag, addr, cache[i].tag_length) == 0){
            if(cache[i].valid == 1){
                total_hits++;
                strcpy(cache[i].tag, addr);
                cache[i].tag_length = calculate_tag_length(addr);
                recents_update(i, cache[i].set);
                return;
            }
        }
    }
    total_misses++;
    total_reads++;
    index = fetch_index(addr, calculate_tag_length(addr)) * assoc;
    for(int i=index; i < index + assoc; i++){
        if(cache[i].valid == 0){
            cache[i].valid = 1;
            strcpy(cache[i].tag, addr);
            cache[i].tag_length = calculate_tag_length(addr);
            recents_update(i, cache[i].set);
            return;
        }
    }
    index = fetch_index(addr, calculate_tag_length(addr)) * assoc;
    for(int i=index; i < index + assoc; i++){
        if(cache[i].recent > max){
            max = cache[i].recent;
            max_index = i;
        }
    }
    cache[max_index].tag_length = calculate_tag_length(addr);
    strcpy(cache[max_index].tag, addr);
    recents_update(max_index, cache[max_index].set);
}

void cache_read(char addr[])
{
    int index = fetch_index(addr, calculate_tag_length(addr)) * assoc;
    int max_index = index;
    int max = 0;
    for(int i=index; i < index + assoc; i++){
        if(cache[i].valid == 1){
            if(strncmp(cache[i].tag, addr, cache[i].tag_length) == 0){
                total_hits++;
                recents_update(i, cache[i].set);
                return;
            }
        }
    }
    total_misses++;
    total_reads++;
    index = fetch_index(addr, calculate_tag_length(addr)) * assoc;
    for(int i=index; i < index + assoc; i++){
        if(cache[i].valid == 0){
            cache[i].valid = 1;
            cache[i].tag_length = calculate_tag_length(addr);
            strcpy(cache[i].tag, addr);
            recents_update(i, cache[i].set);
            return;
        }
    }
    index = fetch_index(addr, calculate_tag_length(addr)) * assoc;
    for(int i=index; i < index + assoc; i++){
        if(cache[i].recent > max){
            max = cache[i].recent;
            max_index = i;
        }
    }
    cache[max_index].tag_length = calculate_tag_length(addr);
    strcpy(cache[max_index].tag, addr);
    recents_update(max_index, cache[max_index].set);
}

void cache_print()
{
    for(int i = 0; i < lines_cnt; i++){
        //printf("index: %d set: %d valid: %d recent: %d tag length: %d tag:", i, cache[i].set, cache[i].valid, cache[i].recent, cache[i].tag_length);
        for(int j = 0; j < cache[i].tag_length; j++){
            //printf("%c", cache[i].tag[j]);
        }
        //printf("\n");
    }
}

void free_cache()
{
    for(int i = 0; i < lines_cnt; i++){
        free(cache[i].tag);
    }
    free(cache);
}

void free_char_ptr(char **x) {
  free(*x);
  *x = NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 5){
        printf("Total 4 Arguments needed.\n");
        return 1;
    }

	//get cache size
    int input_cache_size = atoi(argv[1]);
    if(!((input_cache_size != 0) && (input_cache_size & (input_cache_size - 1)) == 0)){
        printf("Cache size must be a power of 2 and Non-Zero.\n");
        return 1;
    }

	//get blcok size
    int input_block_size = atoi(argv[3]);
    if(!((input_block_size != 0) && ((input_block_size & (input_block_size - 1)) == 0))){
        printf("Block size must be a power of 2 and Non-Zero.\n");
        return 1;
    }else{
        for(int i = 1; i < input_block_size; i*=2){
            block_sub++;
        }
    }

	//get associativity
    if(strcmp(argv[2], "direct") == 0){
        assoc = 1;
    }else if(strcmp(argv[2], "assoc") == 0){
        assoc = (int)(input_cache_size/input_block_size);
    }else{
        char* c = strtok(argv[2], ":");
        c = strtok(NULL, ":");
        if(c == NULL){
            printf("Invalid Associativity provided.\n");
            return 1;
        }
        assoc = atoi(c);
    }

    lines_cnt = (int)(input_cache_size/input_block_size);
    for(int i = 1; i < (lines_cnt / assoc); i*=2){
        set_sub++;
    }

    cache = cache_create();
    if(lines_cnt / assoc * assoc * input_block_size != input_cache_size || input_cache_size == 0){
        printf("Invalid cash-size, block-size or associativity.\n");
        return 1;
    }

    FILE *file = fopen(argv[4], "r");
    if(file == NULL){
        printf("File not found.\n");
        return 1;
    }

    //char wr[2], address[20], *binary;//[50];
    char wr[2], address[20], bin_add[100];

    while (fscanf(file, "%s %s", wr, address) != EOF)
    {
		strcpy(bin_add,address);
		//binary = convert_binary_address(address);
		convert_binary_address(address, bin_add);
		//printf("binary address:%s\n", binary);
		if(strcmp(wr, "W") == 0){
			cache_write(bin_add);
		}else if(strcmp(wr, "R") == 0){
			cache_read(bin_add);
		}
    }
    //free(binary);
	//free_char_ptr(&binary);

	fclose(file);

/*
    printf("Memory total_reads: %d\n", total_reads);
    printf("Memory total_writes: %d\n", total_writes);
    printf("Cache hits: %d\n", total_hits);
    printf("Cache misses: %d\n", total_misses);
*/
    printf("memread:%d\n", total_reads);
    printf("memwrite:%d\n", total_writes);
    printf("cachehit:%d\n", total_hits);
    printf("cachemiss:%d\n", total_misses);

	//free up the cache before leaving
    free_cache();
    return 0;
}
