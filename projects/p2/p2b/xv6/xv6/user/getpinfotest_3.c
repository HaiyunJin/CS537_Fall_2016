
int
main(int argc, char *argv[])
{
    spin(1); // go to level 1
//    printf(1,"blabal\n");
    if (fork() == 0) {
        spin(100);
        exit();
    } else {
        sleep(50); // stay at 1, wait child to 3
        spin(100);
    }
    exit();
    return 0;
}

