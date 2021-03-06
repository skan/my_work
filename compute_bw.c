#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG_START_END_PICTURE 1
#define DEBUG_DISPLAY_ALL_PARSED_TM 1
#define DEBUG_COMPUTE_DOFID 1
#define DEBUG_100_PICT_RESULTS 0
#define DEBUG_MEAN_5_PICT 0

#define TAILLE_MAX 1000 
#define CSV_COLUMN_NUMBER 20
#define DEBUG 5

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
   int    PaceStart;
   int    TotalPace;
}stream_params_t;

typedef struct overall_results_s
{
   int    Dofid;
   int    Pace;
   int    Tranfert;
   float  Bandwidth;
   float  MeanPacesOver5;
   float  MeanBytesOver5;
}overall_results_t;

typedef struct avg_over_5_s
{
   int     bytes;
   int     paces;
}avg_over_5_t;

/******************* Global ***********************/
char                 parsed_line [CSV_COLUMN_NUMBER][200];

/**************** Prototypes **********************/
int parse_current_line (char* line);
int compute_dofid (void);

/**************** Main fct ************************/
int main (int argc, char *argv[])
{

   FILE                 *fichier = NULL;
   char                 chaine[TAILLE_MAX]   = ""; 
   char                 *pch;
   int                  i                    = 0;
   int                  j                    = 0;
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
   int                  TabDofid [2000]      ={0};
   int                  TempDofid            = 9;
   picture_params_t     picture;
   stream_params_t      stream;
   overall_results_t    result [2000];
   avg_over_5_t         avg[5];
   unsigned long long   GlobalTranfer        = 0;
   unsigned long long   CumulatedBytes       = 0;
   unsigned long long   TabCumulatedBytes[20] = {0};
   int                  TabCumulatedPaces[20] = {0};
   unsigned int         PacesOver5            = 0;
   unsigned int         BytesOver5            = 0;

   memset (&picture,0,sizeof(picture_params_t));
   memset (&stream,0,sizeof(stream_params_t));
   memset (&result,0,2000*sizeof(overall_results_t));
   memset (&avg,0,5*sizeof(avg_over_5_t));

   printf ("%s\n",argv[1]);
   fichier = fopen(argv[1], "r");

   if (fichier != NULL)
   {
      while ((fgets(chaine, TAILLE_MAX, fichier) != NULL) & !IsFpfEndFound) 
      {
#if (DEBUG == 1)
          printf("debug: line num %d: %s",line_counter, chaine);
#endif
          line_counter++;
          parse_current_line (chaine);
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
             if (!strcmp(parsed_line[1] , "FPF"))
             {
                 if (!strncmp(parsed_line[6] , "0x100\n",6))
                 {
                     printf ("FPF start message found\n");
                 }
                 else if (!strncmp(parsed_line[6] , "0x300\n",6))
                 {
                    printf ("FPF stop message found\n");
                    IsFpfEndFound = 1;
                    TabCumulatedBytes[i] = CumulatedBytes;
                 }
                 else
                 {
                    IsDofidFound = 1;
                    Dofid = compute_dofid();
                    TabDofid[j] = Dofid;
                    j++;
                 }
                 if ((Dofid%100) == 10)
                 {
                    if (TempDofid == Dofid)
                       i--;
                    TempDofid = Dofid;
                    TabCumulatedBytes[i] = CumulatedBytes;
                    TabCumulatedPaces[i] = paces;
#if (DEBUG_100_PICT_RESULTS == 1)
                    printf("i = %d\n",i);
                    printf ("dofid = %d, get results\n",Dofid);
                    printf ("TabCumulatedBytes[i] = %lld\n", TabCumulatedBytes[i]); 
#endif
                    i++;
#if (DEBUG == 4)
                    printf("i = %d\n,TabCumulatedBytesl[i-1] = %lld\nTabCumulatedPaces[i-1] = %d\n",i, TabCumulatedBytes[i],TabCumulatedPaces[i]);
#endif
                 }
             }/*if (!strcmp(parsed_line[1] , "FPF"))*/

             else if (!strcmp(parsed_line[1] , "TM"))
             {
                bytes = atoi(parsed_line[8]);
                paces = atoi(parsed_line[3]);
                CumulatedBytes = CumulatedBytes + (unsigned long long)bytes;
#if ((DEBUG == 2) || (DEBUG_DISPLAY_ALL_PARSED_TM == 1))
                printf ("\t\tdebug: TM parsed:  paces & bytes = %d & %d \n",paces, bytes);
#endif
                if(!IsPictureFound)
                {
                   if (bytes == 0)
                      previous_null++;
                }
                //if (((bytes > 0) && (previous_null > 0)) || (IsDofidFound || !(IsPictureFound)))
                if (IsDofidFound && !(IsPictureFound))
                {
                   IsDofidFound = 0;
                   PictureNumber++;
                   IsPictureFound = 1;
                   IsStartOfPicture = 1;
                   previous_null = 0;
#if ((DEBUG == 4) || (DEBUG_START_END_PICTURE == 1))   
                   printf ("\t\t\t\tpicture num %d is found\n", PictureNumber);
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
                   if (bytes > 0)
                   {
                      picture.TotalBytes  = picture.TotalBytes + bytes;
                      picture.PaceEnd = paces;
                      next_null = 0;
                   }
                   else
                   {
                      next_null++;
                      //if (next_null > 4) /*configuration of number of null bytes values to confirm end of picture*/
                      if ((next_null > 5) || IsDofidFound) /*configuration of number of null bytes values to confirm end of picture*/
                      {
                         picture.TotalPace = picture.PaceEnd - picture.PaceStart;
                         picture.Bandwidth = ((float)picture.TotalBytes*1000*1000) / ((float)picture.TotalPace*1024*1024); 
                         GlobalTranfer = GlobalTranfer + (unsigned long long)picture.TotalBytes;
#if ((DEBUG == 2) || (DEBUG_START_END_PICTURE == 1))  
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

                         for (i=0 ; i < 4; i++)
                         {
                            avg[i].paces   = avg[i+1].paces;
                            avg[i].bytes   = avg[i+1].bytes;
#if (DEBUG_MEAN_5_PICT == 1)
                            printf("i = %d\t avg_pace = %d\t avg_byte = %d\n", i, avg[i].paces, avg[i].bytes);
#endif
                         }
                         avg[4].paces   = picture.TotalPace;
                         avg[4].bytes   = picture.TotalBytes;
                         PacesOver5 = 0 ;
                         BytesOver5 = 0 ;
                         for (i=0 ; i < 5; i++)
                         {
                            PacesOver5      += avg[i].paces;
                            BytesOver5      += avg[i].bytes;
#if (DEBUG_MEAN_5_PICT == 1)
                            printf("5 paces = %d\t 5 bytes = %d\n", PacesOver5, BytesOver5);
#endif
                         }
                         result[PictureNumber-1].MeanBytesOver5  = (float)(BytesOver5 / 5);
                         result[PictureNumber-1].MeanPacesOver5  = (float)(PacesOver5 / 5);

                         IsPictureFound      = 0;
                         next_null           = 0;
                         previous_null       = 1;
                         picture.TotalBytes   = 0;
                      }/*if (next_null > 3) */
                   }/*if ((bytes > 0) && (next_null < 2))*/
                }/*if (IsPictureFound)*/
             }/*if (!strcmp(parsed_line[1] , "TM"))*/
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

/*Save parsing details for each stream log file*/
   printf ("LogFileName = %s\n",LogFileName); 
   fichier = fopen(LogFileName, "w");
   if (fichier != NULL)
   {
      fprintf(fichier, "%s,default dofid,", stream_name);
      for (i=0; i<PictureNumber; i++)
      {
         fprintf(fichier, ",%d", result[i].Dofid);
      }
      fprintf(fichier, "\n%s,parsed dofid,", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Dofid);
      }
      fprintf(fichier, "\n%s,bytes,%lld", stream_name,GlobalTranfer);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Tranfert);
      }
      fprintf(fichier, "\n%s,pace,", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Pace);
      }
      fprintf(fichier, "\n%s,bw,", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%f", result[i].Bandwidth);
      }
      fprintf(fichier, "\n%s,mean bytes over 5,", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%.0f", result[i].MeanBytesOver5);
      }
      fprintf(fichier, "\n%s,mean time over 5,", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%.0f", result[i].MeanPacesOver5);
      }
      fclose(fichier);
      printf ("%s saved\n",LogFileName);
   }


#if 1 /*save mean over 5 results for all streams*/
   /*look for DOFID 10*/
   j = 2;
   while (j < PictureNumber)
   {
      if ((result[j].Dofid == 10) || (result[j].Dofid == -4096)) 
      {
         break;
      }
      j++;
   }

   fichier = fopen("overall_mean_over_5_results.csv", "a+");
   if (fichier != NULL)
   {
      fprintf(fichier, "\n%s,mean time over 5,", stream_name);
      for (i=j;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%.0f", result[i].MeanPacesOver5);
      }
      fprintf(fichier, "\n%s,mean bytes over 5,", stream_name);
      for (i=j;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%.0f", result[i].MeanBytesOver5);
      }
      printf("overall_mean_over_5_results.csv saved\n");
      fclose(fichier);
   }
#endif

#if 1 /*append stream results details in same csv file*/
   fichier = fopen("overall_streams_details.csv", "a+");
   if (fichier != NULL)
   {
      fprintf(fichier, "%s,default dofid,", stream_name);
      for (i=0; i<PictureNumber; i++)
      {
         fprintf(fichier, ",%d", result[i].Dofid);
      }
      fprintf(fichier, "\n%s,parsed dofid,", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Dofid);
      }
      fprintf(fichier, "\n%s,bytes,%lld", stream_name,GlobalTranfer);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Tranfert);
      }
      fprintf(fichier, "\n%s,pace,", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%d", result[i].Pace);
      }
      fprintf(fichier, "\n%s,bw,", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%f", result[i].Bandwidth);
      }
      fprintf(fichier, "\n%s,mean bytes over 5,", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%.0f", result[i].MeanBytesOver5);
      }
      fprintf(fichier, "\n%s,mean time over 5,", stream_name);
      for (i=0;i<PictureNumber;i++)
      {
         fprintf(fichier, ",%.0f", result[i].MeanPacesOver5);
      }
      fprintf(fichier, "\n\n");
      printf("overall_sreams_details.csv saved\n");
      fclose(fichier);
   }
#endif

#if 0 /*append bandwidth results per pictures' group in overall results.csv*/
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
#endif
   return 0;
}

/********************************************************************************
 * parse current line
 * *****************************************************************************/
int parse_current_line (char* line)
{
   char  *pch;
   int   i = 0;
   pch = strtok (line," ,");
   while (pch != NULL) /*Parse les champs depuis la ligne*/
   {
      strcpy (parsed_line[i] , pch);
#if (DEBUG == 1)
      printf ("\tdebug: param num %d: %s\n",i, parsed_line[i]);
#endif
      pch = strtok (NULL, " ,");
      i++;
   }
   return 0;
}

/********************************************************************************
 * compute dofid
 * *****************************************************************************/
int compute_dofid (void)
{
   char                 BufferDofid [7];
   int                  Dofid = 0;

   //sprintf(BufferDofid,"%c%c%c%c\t", parsed_line[6][2],parsed_line[6][3],parsed_line[6][4],parsed_line[6][5]);
   sprintf(BufferDofid,"%c%c%c%c\t", parsed_line[6][3],parsed_line[6][4],parsed_line[6][5],parsed_line[6][6]);
   sscanf(BufferDofid, "%x\n", &Dofid);
   Dofid = Dofid - 0x1000;
#if ((DEBUG == 1) || (DEBUG_COMPUTE_DOFID == 1))
   printf ("\t\tdebug: FPF = %s\n", parsed_line[6]);
   //printf(BufferDofid,"%c%c%c%c\n", parsed_line[6][2],parsed_line[6][3],parsed_line[6][4],parsed_line[6][5]);
   printf(BufferDofid,"%c%c%c%c\n", parsed_line[6][3],parsed_line[6][4],parsed_line[6][5],parsed_line[6][6]);
   printf("\t\t\tdebug: Dofid = %d\n", Dofid);
#endif
   return Dofid;
}

