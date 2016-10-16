
int
main(int argc, char *argv[])
{
    spin(50); // go to level 3
//    printf(1,"blabal\n");
    if (fork() == 0) {
        spin(100);
        exit();
    } else {
        spin(20);
        sleep(100); // voluntarily relinquishes the CPU
        spin(100); // hope continue tick
    }
    exit();
    return 0;
}

