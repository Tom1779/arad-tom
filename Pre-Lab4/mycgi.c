#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

extern int errno;

#define MAX 10000
typedef struct
{
  char *name;
  char *value;
} ENTRY;

ENTRY entry[MAX];

int cp(char *source, char *destination)
{
  if (!strcmp(source, destination))
  {
    return 0;
  }

  char buf[4096];

  int source_fd = open(source, O_RDONLY);
  if (-1 == source_fd)
  {
    printf("<p>failed to open source file %s, errno = %d</p>\n", source, errno);
    printf("<p>error: %s</p>\n", strerror(errno));
    return -1;
  }
  int destination_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (-1 == destination_fd)
  {
    printf("<p>failed to open destination file %s, errno = %d</p>\n", destination, errno);
    printf("<p>error: %s</p>\n", strerror(errno));
    return -1;
  }
  ssize_t byte_size;
  while (byte_size = read(source_fd, buf, 4096))
  {
    if (-1 == byte_size)
    {
      printf("<p>failed to read bytes</p>\n");
      return -1;
    }
    write(destination_fd, buf, byte_size);
  }
  close(source);
  close(destination);
  return 0;
}

int cat(char *source)
{
  char buf[4097];

  int source_fd = open(source, O_RDONLY);
  if (-1 == source_fd)
  {
    printf("<p>failed to open source file %s, errno = %d</p>\n", source, errno);
    printf("<p>error: %s</p>\n", strerror(errno));
    return -1;
  }
  ssize_t byte_size;
  printf("<p>");
  while (byte_size = read(source_fd, buf, 4096))
  {
    if (-1 == byte_size)
    {
      printf("<p>failed to read bytes</p>\n");
      return -1;
    }
    buf[byte_size] = 0;
    printf("%s", buf);
  }
  printf("</p>");
  close(source);
  return 0;
}

int ls(const char *dir_name)
{
  DIR *directory;
  struct dirent *contents;
  int files = 0;
  printf("<p>dirname = %s</p>\n", dir_name);
  directory = opendir(dir_name);
  if (!directory)
  {
    printf("<p>failed to read directory %s, errno = %d</p>\n", dir_name, errno);
    printf("<p>error: %s</p>\n", strerror(errno));
    return -1;
  }
  while ((contents = readdir(directory)))
  {
    files++;
    printf("<p>File %3d: %s</p>\n", files, contents->d_name);
  }
  closedir(directory);
  return 0;
}

main(int argc, char *argv[])
{
  int i, m, r;
  char cwd[128];

  m = getinputs();  // get user inputs name=value into entry[ ]
  getcwd(cwd, 128); // get CWD pathname

  printf("Content-type: text/html\n\n");
  printf("<html>\n");
  printf("<head><title>Pre-Lab4</title></head>\n");
  printf("<body bgcolor=\"#12D8F2\" link=\"#330033\" leftmargin=8 topmargin=8>\n");
  printf("<p>pid=%d uid=%d cwd=%s</p>\n", getpid(), getuid(), cwd);

  printf("<H1>Echo Your Inputs</H1>\n");
  printf("<p>You submitted the following name/value pairs:</p>\n");

  for (i = 0; i <= m; i++)
    printf("<p>%s = %s</p>\n", entry[i].name, entry[i].value);
  printf("<p></p>\n");

  /*****************************************************************
   Write YOUR C code here to processs the command
         mkdir dirname
         rmdir dirname
         rm    filename
         cat   filename
         cp    file1 file2
         ls    [dirname] <== ls CWD if no dirname
  *****************************************************************/

  if (!strcmp(entry[0].value, "mkdir"))
  {
    printf("<p>mkdir %s</p>\n", entry[1].value);
    r = mkdir(entry[1].value, 755);
    printf("<p>operation returned: %d</p>\n", r);
  }

  if (!strcmp(entry[0].value, "rmdir"))
  {
    printf("<p>rmdir %s</p>\n", entry[1].value);
    r = rmdir(entry[1].value);
    printf("<p>operation returned: %d</p>\n", r);
  }

  if (!strcmp(entry[0].value, "rm"))
  {
    printf("<p>rm %s</p>\n", entry[1].value);
    r = unlink(entry[1].value);
    printf("<p>operation returned: %d</p>\n", r);
  }

  if (!strcmp(entry[0].value, "cp"))
  {

    printf("<p>cp %s : %s</p>\n", entry[1].value, entry[2].value);
    r = cp(entry[1].value, entry[2].value);
    printf("<p>operation returned: %d</p>\n", r);
  }

  if (!strcmp(entry[0].value, "cat"))
  {
    printf("<p>cat %s</p>\n", entry[1].value);
    r = cat(entry[1].value);
    printf("<p>operation returned: %d</p>\n", r);
  }

  if (!strcmp(entry[0].value, "ls"))
  {
    if (strcmp(entry[1].value, ""))
    {
      printf("<p>ls %s</p>\n", entry[1].value);
      r = ls(entry[1].value);
    }
    else
    {
      printf("<p>ls no parameters</p>\n");
      r = ls(".");
    }
    printf("<p>operation returned: %d</p>\n", r);
  }

  // create a FORM webpage for user to submit again
  printf("<p>------------------ DO IT AGAIN ----------------\n");

  //------ NOTE : CHANGE ACTION to YOUR login name ----------------------------
  printf("<FORM METHOD=\"POST\" ACTION=\"http://69.166.48.15/~arad/cgi-bin/mycgi.bin\">\n");

  printf("Enter command : <INPUT NAME=\"command\"> <P>\n");
  printf("Enter filename1: <INPUT NAME=\"filename1\"> <P>\n");
  printf("Enter filename2: <INPUT NAME=\"filename2\"> <P>\n");
  printf("Submit command: <INPUT TYPE=\"submit\" VALUE=\"Click to Submit\"><P>\n");
  printf("</form>\n");
  printf("------------------------------------------------<p>\n");

  printf("</body>\n");
  printf("</html>\n");
}
