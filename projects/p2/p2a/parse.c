
void parse(char *commands, int *argc, char **argv, int *background) {

    const char *delim = " \t\n";
    char *token;
    int i = 0;

    /* get the first token */
    token = strtok(commands,delim); 
    /* walk through other tokens */
    while ( token != NULL ) {
        /* save the token */
        argv[i++] = token;
        token = strtok(NULL,delim);
    }

//    printf("Start to parse\n");
//    printf("number of argv %d\n",i-1);
    argv[i] = NULL; // argvs ended with NULL
    *argc = i ; // store the argc

    // if the last arg is &, run background, & in the middle is ok
    if ( *argv[i-1] == '&') {
//        printf("                      The last argv is &\n");
        *background = 1;  // set background flag
        argv[i-1] = NULL; // remove the last & arg
        (*argc)--; // decrease argc by 1
    } // TODO
    else {
//        printf("  NOTHING IS CHANGED !!!!\n");
    
    }

}


/* OLD CODE, DOES NOT WORK PROPERLY*/
//void parse(char *commands, char **argv, int *background) {
//    printf("Start to parse\n");
//    int i = 0;
//    //while ( *commands != '\n' ) { // loop before the command line ends
//    //    i++;
//    while ( *commands != '\0' || *commands != '\n') { // original code use \0
//    // For interative mode, the line is ended with '\n'
//    printf("1. Try to find the first none special char\n");
//    printf("*command now is: %d\n", *commands);
//    printf("command now is: %p\n", commands);
//        while ( *commands == ' ' || *commands == '\t' || *commands == '\n' ) {
//            *commands++ = '\0';  // change all specical char to \0
//        }
//    printf("2. Find the first none special char\n");
//        *argv++ = commands; // store the argument
//    printf("3. Start to skip consequence char\n");
//        while ( *commands != '\0' && *commands != ' ' && *commands != '\n' 
//                && *commands != '\t') {
//            commands++; // skip until next special char
//        }
//    printf("4. Finish skip consequence char\n");
//    printf("loop %d\n",i);
//    }
//    printf("loop %d\n",i);
//    //*commands++ = '\0';
//    //*commands = '\0';
//    *argv = NULL;
//
//    // if the last arg is &, run background, & in the middle is ok
//    if ( **(argv-1) == '&') {
////        printf("                      The last argv is &\n");
//        *background = 1;  // set background flag
//        *(argv-1) = NULL; // remove the last & arg
//        //**(argv-1) == '\0';
//    } // TODO
//    else {
////        printf("  NOTHING IS CHANGED !!!!\n");
//    
//    }
//}

