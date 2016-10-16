
int
main(int argc, char *argv[])
{
    spin(100);
//    printf(1,"blabal\n");
    if (fork() == 0) {
        spin(100);
        exit();
    } else {
        spin(100);
    }
    exit();
    return 0;
}

