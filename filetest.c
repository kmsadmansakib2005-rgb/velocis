#include <stdio.h>

int main()
{
    const char *path = "C:/Users/USER/Desktop/Velocis/test.vls";

    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        perror("fopen");
        return 1;
    }

    printf("File opened successfully!\n");

    fclose(file);
    return 0;
}