#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json/cJSON.h"

//will probably delete this later and load from json
#define MAP_WIDTH 60
#define MAP_HEIGHT 30

#define MAX_TILE_LAYERS 2

typedef struct
{
    char* name;
    int x;
    int y;
}GameObject;

typedef struct
{
    int layerCount;
    int width;
    int height;
    int tiles[MAX_TILE_LAYERS][(MAP_WIDTH * MAP_HEIGHT)];
}MapObject;

MapObject map;

GameObject* gameObjects = NULL;
int objectsCount;

void ProcessObjects(cJSON* jArray);
void ProcessTiles(cJSON* jArray);
void SerializeObjects(FILE* f);
void SerializeTiles(FILE* f);

char* readFile(char* path)
{
    char* buffer;
    long length;
    FILE* f = fopen(path, "r");
    if (!f) { printf("File could not be read.\n"); exit(1); }

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = malloc(length);
    memset(buffer, 0, length);
    fread(buffer, 1, length, f);

    fclose(f);

    return buffer;
}

void DeserializeJSON(char* path)
{
    cJSON *root, *layers;
    int layersCount = 0;
    int i;

    char* input = readFile(path);

    root = cJSON_Parse(input);

    layers = cJSON_GetObjectItem(root, "layers");
    layersCount = cJSON_GetArraySize(layers);
    printf("Number of layers detected: %d\n", layersCount);
    
    cJSON* curLayer;
    for (i = 0; i < layersCount; i++)
    {
        curLayer = cJSON_GetArrayItem(layers, i);
        char* type = cJSON_GetObjectItem(curLayer, "type")->valuestring;
        if (strcmp(type, "objectgroup") == 0)
        {
            printf("Reading objects layer.\n");
            cJSON* objectArray = cJSON_GetObjectItem(curLayer, "objects");
            ProcessObjects(objectArray);
        }
        else if (strcmp(type, "tilelayer") == 0)
        {
            printf("Reading tile layer.\n");
            cJSON* tileArray = cJSON_GetObjectItem(curLayer, "data");
            ProcessTiles(tileArray);
        }
    }

    //cJSON_Delete(root); //for whatever reason, deleting this too early nulls the values passed into my arrays
    
}

void Serialize(void)
{
    if (gameObjects == NULL)return;
    FILE* f;
    f = fopen("mapOutput.txt", "w");
    if (!f) { printf("Could not write object files.\n"); exit(1); }

    SerializeTiles(f);
    SerializeObjects(f);

    fclose(f);
}

void SerializeObjects(FILE* f)
{
    int i;

    for (i = 0; i < objectsCount; i++)
        fprintf(f, "%s %d %d\n", gameObjects[i].name, gameObjects[i].x, gameObjects[i].y);
  
}

void SerializeTiles(FILE* f)
{
    int i, j, l;

    for (l = 0;l < map.layerCount; l++)
    {
        for (j = 0; j < MAP_HEIGHT; j++)
        {
            for (i = 0; i < MAP_WIDTH; i++)
            {
                fprintf(f, "%d,", map.tiles[l][j * MAP_WIDTH + i]);
            }
            fprintf(f, "\n");
        }
        fprintf(f, "\n");
    }
    
}

void ProcessObjects(cJSON* jArray)
{
    int arraySize = cJSON_GetArraySize(jArray);
    int i;
    cJSON* item;

    gameObjects = (GameObject*)malloc(sizeof(GameObject) * arraySize);

    for (i = 0; i < arraySize; i++)
    {
        item = cJSON_GetArrayItem(jArray, i);
        gameObjects[i].name = cJSON_GetObjectItem(item, "name")->valuestring;
        gameObjects[i].x = cJSON_GetObjectItem(item, "x")->valueint;
        gameObjects[i].y = cJSON_GetObjectItem(item, "y")->valueint;
        printf("Reading %s %d %d\n", gameObjects[i].name, gameObjects[i].x, gameObjects[i].y);
    }

    objectsCount = arraySize;
}

void ProcessTiles(cJSON* jArray)
{
    int arraySize = MAP_WIDTH * MAP_HEIGHT;
    int i;
    cJSON* item;

    for (i = 0; i < arraySize; i++)
    {
        item = cJSON_GetArrayItem(jArray, i)->valueint;
        map.tiles[map.layerCount] [i] = item;
    }
    map.layerCount++;
}


void FreeObjects(void)
{
    free(gameObjects);
}

int main(int argc, char* argv[])
{
    map.layerCount = 0;
    if (argc == 2)
    {
        printf("Processing %s\n", argv[1]);
        DeserializeJSON(argv[1]);
        Serialize();
        FreeObjects();
    }
      
    
	printf("Program Ended.\n");
	return 0;
}