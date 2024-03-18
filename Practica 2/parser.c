#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

struct data_struct {
    int id;
    char first_name[50];
    char last_name[50];
    char email[50];
};

struct data_struct usuarios[100];

int main(){

    FILE *fp;
    fp = fopen("usuarios.csv", "r");

    char row[1000];
    char *token;

    fgets(row, 1000, fp);

    int n = 0;

    while (feof(fp) != true)
    {
        fgets(row, 1000, fp);

        token = strtok(row, ",");
        usuarios[n].id = atoi(token);

        token = strtok(NULL, ",");
        sprintf(usuarios[n].first_name, "%s", token);

        token = strtok(NULL, ",");
        sprintf(usuarios[n].last_name, "%s", token);

        token = strtok(NULL, ",");
        sprintf(usuarios[n].email, "%s", token);

        n++;
    }
    printf("Empieza aqui \n");
    for(int i = 0; i < n; i++){
        printf("Id: %d   Nombre: %s   Apellido: %s   Email: %s", 
            usuarios[i].id, usuarios[i].first_name, usuarios[i].last_name, usuarios[i].email);
    }
    printf("Termina aqui \n");
    return 0;
}