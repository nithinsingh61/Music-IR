

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

    #define sqr(x) ((x)*(x))
    #define max_iterations 10000
    #define buffer_size 10000
    #define learningrateinitial 0.3
    #define neighbourhoodinitial 20

int ninput,no_node,noutput;
int m;
double **train; //m*ninput
//double *input;
double **distance_input; //no_node*no_node
double **distance_node; //no_node*no_node
double ***weights; //no_node*no_node*ninput
double learningrate;
double neighbourhood;
static int fileindex=0;

int **countarr;

char filename[100];


double **mean,**std;
double least,secondleast;
int **winpos;
int *classify;


double randomcoeff(){

            int k=rand()%RAND_MAX;
            double k1;
            //if(k%2==0){
                k1=(double)k/RAND_MAX;
            //}else{
                //k1=(double)(k-RAND_MAX)/RAND_MAX;
            //}
            return k1;
}

double gaussianFunction(double z,double r){
    double k=z/(2*r*r);
    k=exp(-k);
    return k;
}

double* findMax(double **arr,int rows,int cols){
		double *max;
		max=(double *)calloc(cols,sizeof(double));
		int i,j;
		for(j=0;j<cols;j++){
			for(i=1;i<rows;i++){
			if(arr[i][j]>max[j])
			max[j]=arr[i][j];
			}
		}
		return max;

	}

	double* findMin(double **arr,int rows,int cols){
		double *min;
		min=(double*)calloc(cols,sizeof(double));
		int i,j;
		for(j=0;j<cols;j++){
			min[j]=arr[0][j];
			for(i=0;i<rows;i++){
			if(arr[i][j]<min[j])
			min[j]=arr[i][j];
			}
		}
		return min;
	}

	void findMean(){
    int i,j,k;
    double sum=0;
    mean=(double**)calloc(no_node,sizeof(double*));
    for(i=0;i<no_node;i++){
        mean[i]=(double*)calloc(no_node,sizeof(double));
        for(j=0;j<no_node;j++){
            for(k=0;k<ninput;k++){
                sum+=weights[i][j][k];
            }

            mean[i][j]=sum/ninput;
            sum=0;
        }
    }
}

void findStd(){
    int i,j,k;
    double sum=0;
    std=(double**)calloc(no_node,sizeof(double*));
    for(i=0;i<no_node;i++){
        std[i]=(double*)calloc(no_node,sizeof(double));
        for(j=0;j<no_node;j++){
            for(k=0;k<ninput;k++){
                sum+=(weights[i][j][k]-mean[i][j])*(weights[i][j][k]-mean[i][j]);
            }

            std[i][j]=sqrt(sum/ninput);
            sum=0;
        }
    }
}


double** normalise(double** arr,int rows,int cols){
		int i,j;
		double *maxcols;
		maxcols=findMax(arr,rows,cols);
		double *mincols;
		mincols=findMin(arr,rows,cols);

		for(j=0;j<cols;j++){
			for(i=0;i<rows;i++){
			arr[i][j]-=mincols[j];
			arr[i][j]/=(maxcols[j]-mincols[j]);
			}
		}

		return arr;

	}
//change calcDist //pass input[] and weights[][]
	double calcDist(double *pt1,double *pt2){
		double sum=0;
		int i;
		for(i=0;i<ninput;i++){
			sum+=sqr(pt1[i]-pt2[i]);
		}
		return sum;
	}

int countExamples(){

		FILE *fpcsv;
		char buf[buffer_size];
		fpcsv=fopen(filename,"r");

		int co=0;

		while(!feof(fpcsv)){
		co++;
		fgets(buf,sizeof(buf),fpcsv);
		}

		fclose(fpcsv);
		return co;
}

void createNeurons(){
    int i,j;

    distance_input=(double**)calloc(no_node,sizeof(double*));
    for(i=0;i<no_node;i++){
        distance_input[i]=(double*)calloc(no_node,sizeof(double));
    }

    distance_node=(double**)calloc(no_node,sizeof(double*));
    for(i=0;i<no_node;i++){
        distance_node[i]=(double*)calloc(no_node,sizeof(double));
    }

    //input=(double*)calloc(ninput,sizeof(double));



}

void initweights(){
     int i,j,k;
        weights=(double***)calloc(no_node,sizeof(double**));
        for(i=0;i<no_node;i++){
            weights[i]=(double**)calloc(no_node,sizeof(double*));
            for(j=0;j<no_node;j++){
                weights[i][j]=(double*)calloc(ninput,sizeof(double));
                for(k=0;k<ninput;k++){
                    weights[i][j][k]=randomcoeff();
                }
            }
        }
}

void read(){

		FILE *fpcsv;
		//int count=countExamples();
		int count=12700, i,j;
		double ddummy; char c;
		int idummy;
		m=count-1;
		char buf[100];
        //m=(2*count)/3;

   fpcsv=fopen(filename,"r");

   train=(double**)calloc(m,sizeof(double*));

printf("hello");

   for(i=0;i<m;i++){
        train[i]=(double *)calloc(ninput,sizeof(double));
        //fscanf(fpcsv,"%s",buf);
        //fscanf(fpcsv,"%c",&c);
        for(j=0;j<ninput;j++){
            fscanf(fpcsv,"%lf",&ddummy);
            fscanf(fpcsv,"%c",&c);
            train[i][j]=ddummy;

        }
       /* for(j=0;j<noutput;j++){
            fscanf(fpcsv,"%d",&idummy);
            fscanf(fpcsv,"%c",&c);
        }*/
    }


printf("helloHI");

   fclose(fpcsv);
}

void findDist(double *input){
    int i,j;
    for(i=0;i<no_node;i++){
        for(j=0;j<no_node;j++){
            distance_input[i][j]=calcDist(input,weights[i][j]);
        }
    }
}

void winningNode(int *winrow,int *wincol){
    double min=distance_input[0][0];
    int i,j;

    for(i=0;i<no_node;i++){
        for(j=0;j<no_node;j++){
            if(distance_input[i][j]<min){
                min=distance_input[i][j];
                *winrow=i;
                *wincol=j;
            }
        }
    }

}

double* change(double *w,double  *input,double influence,int i,int j){
    int k;
    for(k=0;k<ninput;k++){
        w[k]+=learningrate*influence*(input[k]-weights[i][j][k]);
    }
    return w;
}


void changeWeights(double *input,int no_iters){
    int i,j,k;
    double radiuswidth;
    double influence;
    int winrow=0,wincol=0;
    winningNode(&winrow,&wincol);
        if(no_iters==max_iterations-1){
            //printf("WINNING NODE %d %d \n",winrow,wincol);
        }

    for(i=0;i<no_node;i++){
        for(j=0;j<no_node;j++){
            distance_node[i][j]=sqr((winrow-i))+sqr((wincol-j));
            radiuswidth=neighbourhood;
            if(distance_node[i][j]<radiuswidth*radiuswidth){
                influence=gaussianFunction(distance_node[i][j],radiuswidth);
                weights[i][j]=change(weights[i][j],input,influence,i,j);
            }
        }
    }
}

void changeparam(int no_iters){

        learningrate=learningrateinitial*exp(-no_iters/max_iterations);
        neighbourhood=neighbourhoodinitial*exp(-no_iters/max_iterations);
}

void trainingphase(){
    int count=12700;
    m=count-1;
   // m=(2*count)/3;
    int no_iters=0,i=0;

    while(no_iters<max_iterations){
            if(i==m){
            i=0;
            no_iters++;}
        findDist(train[i]);
        changeWeights(train[i],no_iters);
        changeparam(no_iters);
         i++;
    }
    printf("MAX ITERS %d",no_iters);

}

void printweights(){
    int i,j,k;
    for(i=0;i<no_node;i++){
        for(j=0;j<no_node;j++){
            for(k=0;k<ninput;k++){
                printf("%f ",weights[i][j][k]);
            }
            printf("\n");
        }
        printf("\n");
    }

}

void findLeast(){

    least=std[0][0]<std[0][1]?std[0][0]:std[0][1];
    secondleast=std[0][0]>std[0][1]?std[0][0]:std[0][1];


   /* int i,j,k=0;
    double arr[9];
    for(i=0;i<no_node;i++){
        for(j=0;j<no_node;j++){
           arr[k]=std[i][j];
           k++;
        }
    }
    //sorting
    lcount=0;

    for(i=0;i<no_node*no_node;i++){
        for(j=0;j<no_node*no_node-1-i;j++){
                if(arr[j]>arr[j+1]){
                    double temp=arr[j];
                    arr[j]=arr[j+1];
                    arr[j+1]=temp;
                }
        }
        if(arr[i]<0.06)
            lcount++;
    }
   least=arr[0]; secondleast=arr[1]; thirdleast=arr[2];
    fourthleast=arr[3]; fifthleast=arr[4];
*/
    /*least=(double*)calloc(lcount,sizeof(double));
    for(i=0;i<lcount;i++){
        least[i]=arr[i];
    }*/


int i,j;

   for(i=0;i<no_node;i++){
        for(j=0;j<no_node;j++){
            if(std[i][j]<least){
                secondleast=least;
                least=std[i][j];
            }else if(std[i][j]<secondleast&&std[i][j]>least){
                secondleast=std[i][j];
            }
        }
    }



}

void findClass(){
    int i,j,k;
    int winrow=0,wincol=0;

    winpos=(int**)calloc(m,sizeof(int*));
    classify=(int*)calloc(m,sizeof(int));
    for(i=0;i<m;i++){
        winpos[i]=(int*)calloc(2,sizeof(int));
    }
    int flag=0;

    for(i=0;i<m;i++){
            findDist(train[i]);
            winningNode(&winrow,&wincol);
            winpos[i][0]=winrow;
            winpos[i][1]=wincol;
            printf("%d %d ",winpos[i][0],winpos[i][1]);
            printf("\n");
            if(std[winrow][wincol]==least||std[winrow][wincol]==secondleast)
             classify[i]=1;
            else
                classify[i]=0;
        }

        FILE *fp;
        fp=fopen("StoreBollywood12700AtoO50X5010000iters.txt","a+");
        fprintf(fp,"%s","Next Classification");
        fprintf(fp,"%s","\n");

        for(i=0;i<m;i++){
            fprintf(fp,"%d %d",classify[i],i+1);
            fprintf(fp,"%s","\n");
        }


        fclose(fp);

}


void writeToFile(){
            int i,j,k;

            FILE *fp;
            fp=fopen("SOM_MusicAtoO50X5010000iters.txt","a+");
            fprintf(fp,"%d",fileindex);
            fprintf(fp,"%s","\n");
            fprintf(fp,"%s","No of hidden nodes");
            fprintf(fp,"%d",no_node);
            fprintf(fp,"%s","\n");
            fprintf(fp,"%s","\n");


            fprintf(fp,"%s","Weights");
             for(i=0;i<no_node;i++){
                for(j=0;j<no_node;j++){
                    for(k=0;k<ninput;k++){
                        fprintf(fp,"%lf %s",weights[i][j][k],"\t");
                    }
                    fprintf(fp,"%s","\t");
                }
                fprintf(fp,"%s","\n");
            }


            fprintf(fp,"%s","Weights");


            fprintf(fp,"%s","\n");

            fclose(fp);
            fileindex++;
}

int main(){
    srand(time(NULL));
    printf("Enter filename ");
    scanf("%s",filename);
    printf("Enter no of input ");
    scanf("%d",&ninput);
    printf("Enter no of nodes ");
    scanf("%d",&no_node);

    printf("Enter no of output nodes in file");
    scanf("%d",&noutput);

    read();
    normalise(train,m,ninput);
    createNeurons();
    initweights();
    printweights();

printf("SUP");

    trainingphase();

  printf("ESSENTIAL");
    printweights();
    writeToFile();

    findMean();
    findStd();

    findLeast();
    findClass();

    return 0;
}

