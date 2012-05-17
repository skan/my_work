#include <stdio.h>
#include <string.h>

#define TAILLE_MAX 1000 
#define CSV_COLUMN_NUMBER 20
#define DEBUG 2

typedef struct picture_params_s
{
   int    PaceStart;
   int    PaceEnd;
   int    TotalPace;
   int    TotalBytes;
   float  Bandwidth;
}picture_params_t;

typedef struct stream_params_s
{
   int  PaceStart;
   int  TotalPace;
}stream_params_t;

typedef struct overall_results_s
{
   int  Dofid;
   int  Pace;
   int  Tranfert;
   int  Bandwidth;
}overall_results_t;

//int main ()
int main (int argc, char *argv[])
{

   FILE                 *fichier = NULL;
   char                 chaine[TAILLE_MAX]   = ""; 
   char                 *pch;
   char                 parsed_line [CSV_COLUMN_NUMBER][200];
   int                  i                    = 0;
   int                  line_param           = 0;
   int                  line_counter         = 0;
   int                  bytes                = 0;
   int                  paces                = 0;
   char                 stream_name [200];
   char                 LogFileName [200];
   int                  previous_null        = 0;
   int                  next_null            = 0;
   int                  IsPictureFound       = 0;
   int                  IsStartOfPicture     = 0;
   int                  IsStartOfStream      = 0;
   int                  IsFpfEndFound        = 0;
   int                  IsDofidFound         = 0;
   int                  PictureNumber        = 0;
   int                  Dofid                = 0;
   int                  TempDofid            = 9;
   char                 BufferDofid [7];
   picture_params_t     picture;
   stream_params_t      stream;
   overall_results_t    result [2000];
   unsigned long long   GlobalTranfer        = 0;
   unsigned long long   CumulatedBytes       = 0;
   unsigned long long   TabCumulatedBytes[20] = {0};
   int                  TabCumulatedPaces[20] = {0};

   printf ("%s\n",argv[1]);
   fichier = fopen(argv[1], "r");

   if (fichier != NULL)
   {
      while ((fgets(chaine, TAILLE_MAX, fichier) != NULL) & !IsFpfEndFound) 
      {
          line_counter++;
#if (DEBUG == 1)
          printf("debug: line num %d: %s",line_counter, chaine);
#endif
          pch = strtok (chaine," ,");
          line_param = 0;
          while (pch != NULL) /*Parse les champs depuis la ligne*/
          {
             strcpy (parsed_line[line_param] , pch);
#if (DEBUG == 1)   
             printf ("\tdebug: param num %d: %s\n",line_param, parsed_line[line_param]);
#endif
             pch = strtok (NULL, " ,");
             line_param++;
          }
          if (line_counter < 5) /*skip the 4 first lines (test description)*/
          {
             if (line_counter == 2)/*Recup stream name for results log files*/
             {
                strcpy (stream_name , parsed_line[1]);
                printf ("stream parsed: %s\n", stream_name);
             }
          }
          else
          {
             if (!strcmp(parsed_line[1] , "TM"))
             {
                 bytes = atoi(parsed_line[8]);
                 paces = atoi(parsed_line[4]);
                 CumulatedBytes = CumulatedBytes + (unsigned long long)bytes;
#if (Debug == 1)                 
                 printf("CumulatedBytes = %lld\n", CumulatedBytes); 
#endif
             }
             if (!strcmp(parsed_line[1] , "FPF"))
             {
                 sprintf(BufferDofid,"%c%c%c%c\t", parsed_line[6][2],parsed_line[6][3],parsed_line[6][4],parsed_line[6][5]);
                 sscanf(BufferDofid, "%x\n", &Dofid);
                 Dofid = Dofid - 0x1000;
#if (DEBUG == 1)
                 printf ("\t\tdebug: FPF = %s\n", parsed_line[6]);
                 printf(BufferDofid,"%c%c%c%c\n", parsed_line[6][2],parsed_line[6][3],parsed_line[6][4],parsed_line[6][5]);
                 printf("\t\t\tdebug: Dofid = %d\n", Dofid);
#endif
                 if (!strncmp(parsed_line[6] , "0x100\n",6))
                 {
                     printf ("FPF start message found\n");
                 }
                 else if ((Dofid%100) == 10)
                 {
                    if (TempDofid == Dofid)
                       i--;
                    TempDofid = Dofid;
                    printf("i = %d\n",i);
                    printf ("dofid = %d, get results\n",Dofid);
                    TabCumulatedBytes[i] = CumulatedBytes;
                    printf ("TabCumulatedBytes[i] = %lld\n", TabCumulatedBytes[i]); 
                    TabCumulatedPaces[i] = paces;
                    i++;
#if (DEBUG == 4)
                    printf("i = %d\n,TabCumulatedBytesl[i-1] = %lld\nTabCumulatedPaces[i-1] = %d\n",i, TabCumulatedBytes[i],TabCumulatedPaces[i]);
#endif
                 }
                 else if (!strncmp(parsed_line[6] , "0x300\n",6))
                 {
                    printf ("FPF stop message found\n");
                    IsFpfEndFound = 1;
                    TabCumulatedBytes[i] = CumulatedBytes;
                 }
             }
             else if (!strcmp(parsed_line[1] , "TM"))
             {
                bytes = atoi(parsed_line[8]);
                paces = atoi(parsed_line[3]);
#if (DEBUG == 2)
                printf ("\t\tdebug: TM parsed:  paces & bytes = %d & %d \n",paces, bytes);
#endif
                if(!IsPictureFound)
                {
                   if (bytes == 0)
                      previous_null++;
                }
                if ((bytes > 0) && (previous_null > 0))
                {
                   PictureNumber++;
                   IsPictureFound = 1;
                   IsStartOfPicture = 1;
                   previous_null = 0;
#if (DEBUG == 4)   
                   printf ("\t\t\tpicture num %d is found\n", PictureNumber);
#endif
                }
                if (IsPictureFound)
                {
                   if (IsStartOfStream)
                   {
                      stream.PaceStart = paces;   
                      IsStartOfStream = 0;
                   }
                   if (IsStartOfPicture)
                   {
                      picture.PaceStart = paces;
                      IsStartOfPicture = 0;
                   }
                   if ((bytes > 0) && (next_null < 2))
                   {
                      picture.TotalBytes  = picture.TotalBytes + bytes;
                      picture.PaceEnd = paces;
                      next_null = 0;
                   }
                   else
                   {
                      next_null++;
                      if (next_null > 3) /*Configuration du nombre de 0 pour confirmer la fin de l'image*/
                      {
                         picture.TotalPace = picture.PaceEnd - picture.PaceStart;
                         picture.Bandwidth = ((float)picture.TotalBytes*1000*1000) / ((float)picture.TotalPace*1024*1024); 
                         GlobalTranfer = GlobalTranfer + (unsigned long long)picture.TotalBytes;
#if (DEBUG == 2)   
                         printf ("\t\t\t\t end of picture. Pace End %d & Pace Stard %d\n", picture.PaceEnd, picture.PaceStart);
                         printf ("\t\t\t\t total pace  = %d\n",picture.TotalPace);
                         printf ("\t\t\t\t total bytes = %d\n",picture.TotalBytes);
                         printf ("\t\t\t\t Bandwitdh   = %2f\n", picture.Bandwidth);
                         printf ("\t\t\t\t Global transfer   = %lld\n", GlobalTranfer);
                         printf ("\t\t\t\t Dofid = %d\n", Dofid);

#endif
                         result[PictureNumber-1].Bandwidth = picture.Bandwidth;
                         result[PictureNumber-1].Pace      = picture.TotalPace;
                         result[PictureNumber-1].Tranfert  = picture.TotalBytes;
                         result[PictureNumber-1].Dofid     = Dofid;

                         IsPictureFound      = 0;
                         next_null           = 0;
                         previous_null       = 1;
                         picture.TotalBytes   = 0;
                      }
                   }
                }
             }
          }
       }
   }

   stream.TotalPace = picture.PaceEnd - stream.PaceStart;

   fclose(fichier);
   printf ("Parsing completed, log result to csv file\n");
   strcpy(LogFileName,stream_name);
   pch = stream_name;
   while (*pch)
       pch++;
   --pch;
   strcpy((char*)pch," ");

   pch = LogFileName;
   while (*pch)
       pch++;
   --pch;
   strcpy((char*)pch,"_details.txt");
/*Save parsing details in log file*/
   printf ("LogFileName = %s",LogFileName); 
   fichier = fopen(LogFileName, "w");
   if (fichier != NULL)
   {
      fprintf(fichier, "%s", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Dofid);
      }
      fprintf(fichier, "\n%s,%lld", stream_name,GlobalTranfer);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Tranfert);
      }
      fprintf(fichier, "\n%s,%lld", stream_name,GlobalTranfer);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Pace);
      }
      fprintf(fichier, "\n%s,%lld", stream_name,GlobalTranfer);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Bandwidth);
      }
      fclose(fichier);
      printf ("saved\n");
   }

/*All previous results in same file*/
   fichier = fopen("16bits_with_cache.csv", "a+");
   if (fichier != NULL)
   {
      fprintf(fichier, "%s", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Dofid);
      }
      fprintf(fichier, "\n%s,%lld", stream_name,GlobalTranfer);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Tranfert);
      }
      fprintf(fichier, "\n%s,%lld", stream_name,GlobalTranfer);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Pace);
      }
      fprintf(fichier, "\n%s,%lld", stream_name,GlobalTranfer);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Bandwidth);
      } 
      fprintf(fichier, "\n\n");
      printf("16bits_with_cache.csv saved");
      fclose(fichier);
   }
 

/*add stream resuls to overall result file*/
   fichier = fopen("overall_result.csv", "a+");
   if (fichier != NULL)
   {
      if (fichier != NULL)
      {
         fprintf(fichier, "%s,",stream_name);
         for (i = 0; i < 10; i++)
         {
            fprintf(fichier, "%lld,%d,",TabCumulatedBytes[i],TabCumulatedPaces[i]);
         }
         fprintf(fichier, "%lld,%d,%d\n",CumulatedBytes,PictureNumber,stream.TotalPace);
      }
      printf("overall_restult.csv saved");
      fclose(fichier);
   }
   return 0;
}

#if 0
   char str[] ="val1, val2,val3,- This, a sample string.";
   char * pch;
   ;printf ("Splitting string \"%s\" into tokens:\n",str);
   printf ("Splitting string \"%s\" into tokens:\n",chaine);
   pch = strtok (chaine,",");
   while (pch != NULL)
   {
       printf ("%s\n",pch);
      ; pch = strtok (NULL, " ,.-");
   }
#endif
