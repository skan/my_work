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
   int                  IsPictureFound       = 0;
   int                  previous_null        = 0;
   int                  next_null            = 0;
   int                  IsStartOfPicture     = 0;
   int                  IsStartOfStream      = 0;
   int                  IsFpfEndFound        = 0;
   int                  PictureNumber        = 0;
   picture_params_t     picture;
   stream_params_t      stream;
   overall_results_t    result [2000];
   unsigned long long   GlobalTranfer        = 0;
   char                 BufResult[10000];
   unsigned long long   CumulatedBytes       = 0;
   unsigned long long   TabCumulatedBytes[10] = {0};
   int                  TabCumulatedPaces[10] = {0};
   int                  Dofid                = 0;
   char                 BufferDofid [7];
   int                  IsDofidFound         = 0;

#if 0
   fichier = fopen("Data_Sbag_11-05-12_21-39-43.csv", "r");
#else
   printf ("%s\n",argv[1]);
#endif
   fichier = fopen(argv[1], "r");

   if (fichier != NULL)
   {
       while ((fgets(chaine, TAILLE_MAX, fichier) != NULL) & !IsFpfEndFound) 
       {     
          line_counter++;
#if (DEBUG == 3)   
          printf("line num %d: %s",line_counter, chaine); 
#endif
          pch = strtok (chaine," ,");
          line_param = 0;
          while (pch != NULL) /*Parse les champs depuis la ligne*/
          {
             strcpy (parsed_line[line_param] , pch);
#if (DEBUG == 3)   
             printf ("param num %d: %s\n",line_param, parsed_line[line_param]);
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
#if 1 /*Compute Cumulated bytes*/          
             if (!strcmp(parsed_line[1] , "TM"))
             {
                 bytes = atoi(parsed_line[8]);
                 paces = atoi(parsed_line[4]);
                 CumulatedBytes = CumulatedBytes + (unsigned long long)bytes;
             }
#endif          
             if (!strcmp(parsed_line[1] , "FPF"))
             {
#if (DEBUG == 5)
                 printf ("debug: FPF value is %s\n", parsed_line[6]);
#endif
#if 1 /*Parse Dofid's picture*/
                 sprintf(BufferDofid,"%c%c%c%c\t", parsed_line[6][2],parsed_line[6][3],parsed_line[6][4],parsed_line[6][5]);
                 sscanf(BufferDofid, "%x\n", &Dofid);
                 Dofid = Dofid - 0x1000;
#if (DEBUG == 2)
                 printf(BufferDofid,"%c%c%c%c\t", parsed_line[6][2],parsed_line[6][3],parsed_line[6][4],parsed_line[6][5]);
                 printf("Dofid = %d\n", Dofid);
#endif
#endif
                 if (!strncmp(parsed_line[6] , "0x100\n",6))
                 {
                     printf ("FPF start message found\n");
                 }
                 else if (Dofid == 110)
                 {
                    printf ("FPF 110\n");
                    TabCumulatedBytes[0] = CumulatedBytes;
                    TabCumulatedPaces[0] = paces;
                 }
                 else if (Dofid == 210)
                 {
                    printf ("FPF 200\n");
                    TabCumulatedBytes[1] = CumulatedBytes;
                    TabCumulatedPaces[1] = paces;
                 }
                 else if (Dofid == 310)
                 {
                    printf ("FPF 300\n");
                    TabCumulatedBytes[2] = CumulatedBytes;
                    TabCumulatedPaces[2] = paces;
                 }
                 else if (Dofid == 410)
                 {
                    printf ("FPF 400\n");
                    TabCumulatedBytes[3] = CumulatedBytes;
                    TabCumulatedPaces[3] = paces;
                 }
                 else if ((Dofid == 510) || (!strncmp(parsed_line[6] , "0x500\n",6)))
                 {
                    printf ("FPF 500\n");
                    TabCumulatedBytes[4] = CumulatedBytes;
                    TabCumulatedPaces[4] = paces;
                 }
                 else if (Dofid == 610)
                 {
                    printf ("FPF 600\n");
                    TabCumulatedBytes[5] = CumulatedBytes;
                    TabCumulatedPaces[5] = paces;
                 }
                 else if ((Dofid == 710) || (!strncmp(parsed_line[6] , "0x700\n",6)))
                 {
                    printf ("FPF 700\n");
                    TabCumulatedBytes[6] = CumulatedBytes;
                    TabCumulatedPaces[6] = paces;
                 }
                 else if (Dofid == 810)
                 {
                    printf ("FPF 700\n");
                    TabCumulatedBytes[7] = CumulatedBytes;
                    TabCumulatedPaces[7] = paces;
                 }
                 else if ((Dofid == 910) || (!strncmp(parsed_line[6] , "0x900\n",6)))
                 {
                    printf ("FPF 900\n");
                    TabCumulatedBytes[8] = CumulatedBytes;
                    TabCumulatedPaces[8] = paces;
                 }
                 else if (Dofid == 1010)
                 {
                    printf ("FPF 1000\n");
                    TabCumulatedBytes[9] = CumulatedBytes;
                    TabCumulatedPaces[9] = paces;
                 }
                 else if (!strncmp(parsed_line[6] , "0x300\n",6))
                 {
                    printf ("FPF stop message found\n");
                    IsFpfEndFound = 1;
                    TabCumulatedBytes[4] = CumulatedBytes;
                 }
             }
             else if (!strcmp(parsed_line[1] , "TM"))
             {
                bytes = atoi(parsed_line[8]);
                paces = atoi(parsed_line[3]);
#if (DEBUG == 2)
                printf ("\t\tpaces & bytes = %d & %d \n",paces, bytes);
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
#if (DEBUG == 2)   
                   printf ("picture num %d is found\n", PictureNumber);
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
                         printf ("\t end of picture. Pace End %d & Pace Stard %d\n", picture.PaceEnd, picture.PaceStart);
                         printf ("\t total pace  = %d\n",picture.TotalPace);
                         printf ("\t total bytes = %d\n",picture.TotalBytes);
                         printf ("\t Bandwitdh   = %2f\n", picture.Bandwidth);
                         printf ("\t Global transfer   = %lld\n", GlobalTranfer);
                         printf ("\t Dofid = %d\n", Dofid);

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
   printf ("LogFileName = %s\n",LogFileName); 
/*Save parsing details in log file*/
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
      fclose(fichier);
   }
 

/*add stream resuls to overall result file*/
   fichier = fopen("overall_result.csv", "a+");
   if (fichier != NULL)
   {
      printf ("overall results in: overall_result.csv\n");
      if (fichier != NULL)
      {
         //fprintf(fichier, "%d,%s,%u,%d,%d\n",IsFpfEndFound, stream_name,GlobalTranfer,PictureNumber,StreamTotalPace);
         //fprintf(fichier, "%d,%s,%lld,%d,%d\n",IsFpfEndFound, stream_name,CumulatedBytes,PictureNumber,StreamTotalPace);
         fprintf(fichier, "%s,%lld,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d,%lld,%d,%d\n",stream_name,
                                                                                 TabCumulatedBytes[0],TabCumulatedPaces[0],TabCumulatedBytes[1],TabCumulatedPaces[1],TabCumulatedBytes[2],TabCumulatedPaces[2],
                                                                                 TabCumulatedBytes[3],TabCumulatedPaces[3],TabCumulatedBytes[4],TabCumulatedPaces[4],TabCumulatedBytes[5],TabCumulatedPaces[5],
                                                                                 TabCumulatedBytes[6],TabCumulatedPaces[6],TabCumulatedBytes[7],TabCumulatedPaces[7],TabCumulatedBytes[8],TabCumulatedPaces[8],
                                                                                 TabCumulatedBytes[9],TabCumulatedPaces[9],CumulatedBytes,PictureNumber,stream.TotalPace);
      }
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
