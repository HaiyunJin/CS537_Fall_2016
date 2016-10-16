
int
main(int argc, char *argv[])
{
    spin(100); // go to level 3
//    printf(1,"blabal\n");
   int i;
   for (i = 0; i < 8; i++) {
    if (fork() == 0) {
        spin(1000);
        exit();
    } 
   }
//    else {
//         sleep(50); // voluntarily relinquishes the CPU
//         spin(100); // hope continue tick
//     }
    exit();
    return 0;
}

