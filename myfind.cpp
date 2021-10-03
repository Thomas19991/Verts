#include <stdio.h>
#include <iostream>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h> // thats for fork()
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <getopt.h>
#include <assert.h>
// #define ___STDC_WANT_LIB_EXIT1___ 1
#include <sys/stat.h> // enthält s_isdir !!

void searchFor(char const *listName, std::string name, bool caseSensitive, bool *found);
void recursiveSearch(char const *listName, std::string name, bool caseSensitive, bool *found);

bool isList(char const *path);

//argc represents the number of things, that have been entered in the command line when running program in terminal
//argv is the array that holds the string values of the things that were entered on the command line on the program.
//|| in C!! char* is a string so an array of strings
// argCount argVector (stores in Vector)
int main(int argc, char **argv)
{
    char c;
    //int isAEnabled = 0;
    bool R_File = false;
    bool i_File = false;

    std::string files[argc - optind - 1];
    std::string inList;
    int status;
    // go throug all flags ||The getopt () function returns different values; ___parse command line arguments.
    while ((c = getopt(argc, argv, "iR")) != -1)
    //i: dann wäre i required !! || not nessesary
    {
        switch (c)
        {
        case 'R': //should switch myfind in recursive mode
            R_File = true;
            break;
        case 'i': // case in-sensitive search
            i_File = true;
            break;
        case '?':
            std::cout << "illegal argument" << std::endl;
        default:
            std::cout << "Expression does not match any constant." << std::endl;
        }
    }
    // optind is the variable, which are no longer options in the vector
    //mycommandcode.c
    for (int i = optind; i < argc; i++)
    {

        if (i == optind)
        {
            inList = argv[i];
        }
        else
        {
            files[i - optind - 1] = argv[i];
        }
    }
    // message print what the programm has to do
    std::cout << "Searching in List: '" << inList << "' for files: ";
    for (int i = 0; i < argc - optind - 1; i++)
    {
        std::cout << "'" << files[i] << "'" << std::endl;
    }

    for (int i = optind + 1; i < argc; i++) // gets file counter and iterates through files
    {
        // fork() a child process
        pid_t pid = fork();
        switch (pid)
        {
        // error handling
        case -1:
            std::cout << "Child process err!!!" << std::endl;
            return EXIT_FAILURE; // EXIT_SUCCESS = 0; || -1 ------------same ?? -----
            break;

        // if its childs process turn, start searching
        case 0:
            bool found = false;
            bool *pFound = &found;
            if (R_File)
            {
                recursiveSearch(const_cast<char *>(inList.c_str()), argv[i], i_File, pFound);
            }
            else
            {
                searchFor(const_cast<char *>(inList.c_str()), argv[i], i_File, pFound);
            }
            if (!*pFound)
            {
                //Exception
                if (i_File == 0 || R_File == 0)
                {
                    throw std::invalid_argument("invalid_File not found");
                }

                std::cout << "File not found: " << argv[i] << std::endl;
            }

            return EXIT_SUCCESS;
            break;
        }
        // wait for other child processes, if finished
        if (pid > 0)
        {
            wait(&status);
        }
    }
    return 0;
}

void searchFor(char const *listName, std::string name, bool case_Sensitive, bool *found)
{

    char pathBuf[PATH_MAX + 1];    // PATH_MAX is defined as 1024
    struct dirent *direntp;        //actual file
    DIR *dirp = opendir(listName); // open the given directory

    // if the directory doesn't exist, throw an err
    if (dirp == NULL)
    {
        std::cout << "No file like that actually existing here!!" << std::endl;
        exit(1); // exit true
    }

    //iterate through all files in the list
    while ((direntp = readdir(dirp)) != NULL && *found == false)
    {

        bool compResult = false;
        // if it should be case sensitive compare with strcmp
        if (!case_Sensitive)
        {
            compResult = !strcmp(direntp->d_name, name.c_str());
        }

        else
        { // if not, compare with strcasecmp (without camelcasesensitive!))
            compResult = !strcasecmp(direntp->d_name, name.c_str());
        }

        if (compResult) // if compare was succesfull, print found message with infos
        {

            // gets absolute path
            realpath(direntp->d_name, pathBuf);
            std::cout << "< pid>: " << getpid() << " <filename>: '" << direntp->d_name << "' <complete-path-to-found-file>'" << pathBuf
                      << "\n"
                      << std::endl;
            //mark file as found
            *found = true;
        }
    }

    // this is that there is no zombie processe afterwards
    while ((closedir(dirp) == -1) && (errno == EINTR))
        ; //errno is number of last err // system call was in progress
}

//function for a recursive search
void recursiveSearch(char const *listName, std::string name, bool case_Sensitive, bool *found)
{

    //using Makro to keep space and an automatic Update
    char finalPath[PATH_MAX + 1];
    struct dirent *direntp; //actual file //directory_entry

    // open the in the console entered directory
    DIR *dirp = opendir(listName);

    while ((direntp = readdir(dirp)) != NULL && *found == false)
    {
        std::string type = "";

        memset(finalPath, 0, PATH_MAX + 1); // memset¹sets the first binary number - to the specific number---

        // filejumping
        if (strcmp(direntp->d_name, ".") && strcmp(direntp->d_name, "..")) // if same = 0
        {
            //strcat to concatenate 2 strings
            strcat(finalPath, listName);
            strcat(finalPath, "/");
            strcat(finalPath, direntp->d_name);

            bool compResult = false;
            // if it should be case sensitive compare with strcmp
            if (!case_Sensitive)
            {
                compResult = !strcmp(direntp->d_name, name.c_str());
            }
            else
            {
                //  else -- compare with strcasecmp (without camelcasesensitive!))
                compResult = !strcasecmp(direntp->d_name, name.c_str());
            }
            if (compResult)
            {

                // gets absolute path [entered in our case ./myfind -R . myfind.cpp]
                std::cout << "< pid> :" << getpid() << " <filename>: '" << direntp->d_name << "' <complete-path-to-found-file>'" << finalPath
                          << "'" << std::endl;
                //mark file as found
                *found = true;
            }

            //recursive - call up the current one again
            if (isList(finalPath))
            {
                recursiveSearch(finalPath, name, case_Sensitive, found);
            }
        }
    }

    //opened do not respond - closes the file descriptor
    closedir(dirp);
}

// function isList pulls the pointer back  || fork.c
bool isList(char const *path)
{
    // this function detects if the given path points on a file or a folder
    DIR *dir = NULL;
    // returns the pointer
    dir = opendir(path);
    // if the directory can't be opened, it is no directory
    if (dir == NULL)
    {
        return false;
    }
    else
    {

        // if it was opened, close id and retunr true
        closedir(dir);
        return true;
    }
}
