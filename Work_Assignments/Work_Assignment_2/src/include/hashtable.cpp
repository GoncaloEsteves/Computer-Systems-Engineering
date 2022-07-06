#include <stdio.h>
#include <string.h>
#include <mutex>
#include <atomic>
#include <iostream>
#include <fstream>
#include <time.h>

namespace hasht {
    struct Element {
        long *keys;
        char **values;
        int length;
        int used;
	    std::mutex mut;
    };

    struct HashTable {
        struct Element *table;
        int size;

        HashTable(int N){
            size = N;
            table = new struct Element[N];

            for(auto i = 0; i < N; i++){
                table[i].keys = new long[N];
                table[i].values = new char*[N];
                table[i].length = N;
                table[i].used = 0;
            }
        }

        HashTable(const char* filename){
            std::ifstream myfile;
            std::chrono::high_resolution_clock::time_point t1;
			std::chrono::high_resolution_clock::time_point t2;
			std::chrono::duration<double, std::milli> time_span;

            myfile.open(filename);

            if(!myfile)
                std::cout << "Opening file failed" << std::endl;
            
            char *aux = NULL, *key = NULL;
            std::string line;
            int N = 1024;

  			t1 = std::chrono::high_resolution_clock::now();

            if(getline(myfile, line)){
                aux = strdup(line.c_str());
                N = atoi(aux);
            }

            size = N;
            table = new struct Element[N];

            int i;

            for(i = 0; i < N; i++){
                table[i].keys = new long[N];
                table[i].values = new char*[N];
                table[i].length = N;
                table[i].used = 0;
            }

            int flag = 1, j;
            i = 0;

            while(flag){
                if(!(getline(myfile, line)))
                    flag = 0;

                else{
                    aux = strdup(line.c_str());
                    N = atoi(aux);

                    if(N > table[i].length){
                        free(table[i].keys);
                        free(table[i].values);
                        table[i].keys = new long[N];
                        table[i].values = new char*[N];
                        table[i].length = N;
                    }

                    j = 0;

                    while(j < N && flag){
                        if(!(getline(myfile, line)))
                            flag = 0;

                        else{
                            aux = strdup(line.c_str());

                            key = strdup(strtok(aux, " "));
                            table[i].keys[j] = atoi(key);
                            table[i].values[j] = strdup(strtok(NULL, " "));
                            j++;
                        }
                    }

                    table[i].used = j;
                    i++;
                }
            }
  			
            t2 = std::chrono::high_resolution_clock::now();
 			time_span = t2 - t1;

            printf("Tempo de leitura: %lfms\n", (time_span.count()));
        }

        int hash(long key){
            return (key%size);
        }

        void put(long key, char* value){
            int hash = hasht::HashTable::hash(key);

            std::lock_guard<std::mutex> lock(table[hash].mut);

            if(table[hash].used == table[hash].length){
                long *keysAux = new long[table[hash].used];
                char **valuesAux = new char*[table[hash].used];

                for(auto i = 0; i < table[hash].used; i++){
                    keysAux[i] = table[hash].keys[i];
                    valuesAux[i] = strdup(table[hash].values[i]);
                    free(table[hash].values[i]);
                }

                free(table[hash].keys);
                free(table[hash].values);

                table[hash].length = table[hash].length * 2;
                table[hash].keys = new long[table[hash].length];
                table[hash].values = new char*[table[hash].length];

                for(auto i = 0; i < table[hash].used; i++){
                    table[hash].keys[i] = keysAux[i];
                    table[hash].values[i] = strdup(valuesAux[i]);
                    free(valuesAux[i]);
                }

                free(keysAux);
                free(valuesAux);
            }

            int i;

            for(i = 0; i < table[hash].used; i++){
                if(table[hash].keys[i] == key)
                    break;
            }

            if(i < table[hash].used)
                free(table[hash].values[i]);
            else
                table[hash].used++;

            table[hash].keys[i] = key;
            table[hash].values[i] = strdup(value);
        }

        char* get(int key){
            int hash = hasht::HashTable::hash(key);

            std::lock_guard<std::mutex> lock(table[hash].mut);
            
            char *aux = NULL;
            
            for(auto i = table[hash].used - 1; i >= 0; i--){
                if(table[hash].keys[i] == key){
                    aux = strdup(table[hash].values[i]);
                    break;
                }
            }

            return aux;
        }

        char* createBackup(){

            char date[100];
            time_t temp = time(NULL);
            struct tm *timeptr = localtime(&temp);

            strftime(date, sizeof(date), "backups/%d_%b_%Y-%H_%M_%S.txt", timeptr);

            std::ofstream myfile;

            myfile.open(date);

            if(!myfile)
                std::cout << "Opening file failed" << std::endl;
            
            myfile << size << "\n";

            for(int i = 0; i < size; i++){
                myfile << table[i].used << "\n";
                
                for(int j = 0; j < table[i].used; j++)
                    myfile << table[i].keys[j] << " " << table[i].values[j] << "\n";
            }

            myfile.close();

            return strdup(date);
        }

        void freeTable(){
            for(int i = 0; i < size; i++){
                for(int j = 0; j < table[i].used; j++){
                    free(table[i].values[j]);
                }
                free(table[i].keys);
                free(table[i].values);
            }
            free(table);
        }
    };
}