/***********************************************************************
*	TIMBERWOLF SIMULATED ANNEALING PLACEMENT ALGORITHM
*
***********************************************************************/

/*************************************HEADER FILES***************************************************/
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

/************************************* MACROS ***************************************************/



int NumNodes;
int NumTerminals;
int NumNets;
int NumPins;

/************************************* NODE DATA STRUCTURE ***************************************************/

struct node
{
	char name[10]; // name of the node
	int width;		// width of the node
	int height;		// height of the node
	int terminal;	// stores if node is terminal or not
	int xCenter;	// center x co ordinate
	int yCenter;	// center y co ordinate
	int x;			// left most corner of the node
	int y;			
	int type;
	int rowID;                                   
	int partition;	// stores in which partion the node lies
	
};

struct node Nodes[1000000];
struct node Nodestemp[1000000];
struct node Nodes1[1000000];
struct node Nodes2[1000000];


int NetDegree[40000];
int NetList[40000][25];
int dividegrid_x = 15;
int dividegrid_y = 15;
int xGridrange[10];
int yGridrange[10];
int width_xgrid;
int height_ygrid;


//------------------------------

struct rowNode
{
	int num;
	struct rowNode *next;
};




void setNodeParameter(int num, char* name, int w, int d, char* terminal){
	strcpy(Nodes[num].name, name);
	Nodes[num].width = w;
	Nodes[num].height = d;
	if(strcmp(terminal, "terminal") == 0)	Nodes[num].terminal = 1;
	else Nodes[num].terminal = 0;
	
}
	
void readNodesFile(){
	FILE *fp;
	char buf1[100];
	char buf2[100];
	int num = 0, w, h;
	
	// Open the file
	fp = fopen("ibm01/ibm01.nodes", "r");  
	if (fp == NULL){
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
    }
	
	fscanf(fp, "%s : %d", buf1, &NumNodes);
	fscanf(fp, "%s : %d", buf1, &NumTerminals);
	//printf("Begin Reading Nodes File \nNumNodes = %d, NumTerminals = %d\n", NumNodes, NumTerminals);
	
	for(num = 0; num < NumTerminals; num++) {
		fscanf(fp, "%s %d %d %s\n", buf1, &w, &h, buf2);
		setNodeParameter(num, buf1, w, h, buf2);
		
		//	printf("%d: width %d, height %d\n", num, Nodes[num].width, Nodes[num].height);
	}
	for(num = NumTerminals; num < NumNodes; num++) {
		fscanf(fp, "%s %d %d\n", buf1, &w, &h);
		setNodeParameter(num, buf1, w, h, "");
		
		//	printf("%d: width %d, height %d\n", num, Nodes[num].width, Nodes[num].height);
	}
	fclose(fp);
	

}

void setTerminalParameter(int num, char* name, int xCenter, int yCenter){
	if(strcmp(Nodes[num].name, name) == 0) {
		Nodes[num].xCenter = xCenter;
		Nodes[num].yCenter = yCenter;
	}
	else {
		perror("Error while matching the place information with the node.\n");
	}
}

void readPlFile(){
	FILE *fp;
	char buf1[100];
	int num = 0, x, y;
	

	fp = fopen("ibm01/ibm01.pl", "r"); 
	if (fp == NULL){
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
    }
	

	fscanf(fp, "%s : %d", buf1, &NumTerminals);
	printf("NumTerminals = %d\n", NumTerminals);
	

	for(num = 0; num < NumTerminals; num++) {
		fscanf(fp, "%s %d %d", buf1, &x, &y);
		setTerminalParameter(num, buf1, x, y);
	}
	fclose(fp);
	

}


void readNetsFile(){
	FILE *fp;
	char buf1[100], buf2[100], nameFirstChar;
	int num = 0, degree = 0, i = 0, index = 0;
	char *token;
	

	fp = fopen("ibm01/ibm01.nets", "r");  
	if (fp == NULL){
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
    }
	
	fscanf(fp, "%s : %d\n", buf1, &NumNets);
	fscanf(fp, "%s : %d\n", buf1, &NumPins);
	printf("NumNets = %d, NumPinss = %d\n", NumNets, NumPins);
	while(num < NumNets){
		fscanf(fp, "%s : %d\n", buf1, &degree);
		NetDegree[num] = degree;

		for(i = 0; i < degree; i++){
			fgets(buf1, 100, fp);
			token = strtok(buf1, " ");
			nameFirstChar = *token;
			index = atoi(++token);
			
			NetList[num][i] = (nameFirstChar == 'p') ? (index - 1) : (NumTerminals + index);
			
		}
		num++;
	}
	
}

struct boundary
{
	int minX;
	int maxX;
	int minY;
	int maxY;
};

struct boundary b;

void BoundaryCalculation()
{
	int currentMinX, currentMaxX, currentMinY, currentMaxY;
	int i;
	currentMinX = Nodes[0].xCenter;
	currentMaxX = Nodes[0].xCenter;
	currentMinY = Nodes[0].yCenter;
	currentMaxY = Nodes[0].yCenter;
	for(i = 1; i < NumTerminals; i++){
		if(Nodes[i].xCenter < currentMinX)
			currentMinX = Nodes[i].xCenter;
		else if(Nodes[i].xCenter > currentMaxX)
			currentMaxX = Nodes[i].xCenter;
		if(Nodes[i].yCenter < currentMinY)
			currentMinY = Nodes[i].yCenter;
		else if(Nodes[i].yCenter > currentMaxY)
			currentMaxY = Nodes[i].yCenter;
	}
	b.minX = currentMinX;
	b.maxX = currentMaxX;
	b.minY = currentMinY;
	b.maxY = currentMaxY;
}


void writeGraphFile()
{
	FILE *fp = NULL;
	fp = fopen("graph.txt", "w+");
	if(fp==NULL){
		printf("Cannot open file\n");
	}
	fprintf(fp, "%d %d\n", NumNets, NumNodes);
	int i, j;
	for(i = 0; i < NumNets; i++){
		for(j = 0; j < NetDegree[i]; j++){
			fprintf(fp, "%d ", NetList[i][j]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}


void GridDivision(){
	//int partition = 4;
	int j = 4;
	int grid[4];
	int gap = (b.maxX - b.minX)/4;
	printf("gap %d\n", gap);
	for (int i = 0; i < 4; ++i)
	{
		grid[i] = grid[i] + i * gap;  // 1 899 1800 2682
		printf("grid %d %d\n",i,grid[i]);
	}

}
void calNodecenter(int num, int x, int y){
	Nodes[num].x = x;
	Nodes[num].y = y;
	Nodes[num].xCenter = Nodes[num].x + Nodes[num].width/2;
	Nodes[num].yCenter = Nodes[num].y + Nodes[num].height/2;

}



void initialplacement(){
	FILE *fp = NULL;
	fp = fopen("Test/outputibm01.txt", "w+");
	if(fp==NULL){
		printf("Cannot open file\n");
	}
	fprintf(fp, "NumNodes : %d\n",NumNodes);
	fprintf(fp, "NumTerminals : %d\n",NumTerminals);
	int partition;
	struct node newNode;
	for(int i =0; i<NumNodes; i++){
		if(Nodes[i].terminal==1){
		printf("x %d ",Nodes[i].x);
	    fprintf(fp, "%s %d ",Nodes[i].name ,Nodes[i].xCenter);
	    printf("x %d ",Nodes[i].y);
	    fprintf(fp, "%d\n",Nodes[i].yCenter);
		}
	}
//TAKE CARE OF THIS
	for(int i =0; i<NumNodes; i++){
		if(Nodes[i].terminal==0){
	partition = Nodes[i].partition;
	//while(node.xCenter > b.minX & node.xCenter < b.maxX){
	int x_range = b.maxX - b.minX + 1 - 100;
	//int check_x = 
	//while((Nodes[i].xCenter + Nodes[i].width/2) > b.minX && (Nodes[i].xCenter + Nodes[i].width/2) < b.maxX){   // Take of this problem
	Nodes[i].xCenter = rand() % x_range; // changed x center to x
	if(Nodes[i].xCenter < 0){
	if((Nodes[i].xCenter - Nodes[i].width/2) < b.minY) 
	{
		Nodes[i].xCenter = Nodes[i].xCenter + Nodes[i].width;	
	} 
	else if((Nodes[i].x + Nodes[i].width/2) > b.maxY)
	{
		Nodes[i].xCenter = Nodes[i].xCenter - Nodes[i].width;
	}
    }
    else if (Nodes[i].xCenter > 0)
    {
    if((Nodes[i].xCenter + Nodes[i].width/2) < b.minY) 
	{
		Nodes[i].xCenter = Nodes[i].xCenter + Nodes[i].width;	
	} 
	else if((Nodes[i].x + Nodes[i].width/2) > b.maxY)
	{
		Nodes[i].xCenter = Nodes[i].xCenter - Nodes[i].width;
	}
    }
	//break;
	//}
	printf("x %d ",Nodes[i].xCenter);
	fprintf(fp, "%s %d ",Nodes[i].name ,Nodes[i].xCenter);
	int y_range = b.maxY - b.minY + 1 - 100;
	//while((Nodes[i].yCenter + Nodes[i].height/2) > b.minY && (Nodes[i].yCenter + Nodes[i].height/2) < b.maxY){
	Nodes[i].yCenter = rand() % y_range;  // changed ycenter to y
	if((Nodes[i].yCenter + Nodes[i].height/2) < b.minX)
	{
		Nodes[i].xCenter = Nodes[i].xCenter + Nodes[i].height;
	}
	else if((Nodes[i].yCenter + Nodes[i].height/2) > b.maxX)
		{
		Nodes[i].xCenter = Nodes[i].xCenter - Nodes[i].height;
	}
	//break;
	//}
	printf("y %d\n",Nodes[i].yCenter);
	fprintf(fp, "%d\n", Nodes[i].yCenter);
	}

	}
	
}
/*int NetDegree[40000];
int NetList[40000][25];
int dividegrid_x = 15;
int dividegrid_y = 15;
int xGridrange[10];
int yGridrange[10];
int width_xgrid;
int height_ygrid;
*/
void initialplacement1(){
	FILE *fp = NULL;
	fp = fopen("Test/outputibm01_1.txt", "w+");
	if(fp==NULL){
		printf("Cannot open file\n");
	}
	fprintf(fp, "NumNodes : %d\n",NumNodes);
	fprintf(fp, "NumTerminals : %d\n",NumTerminals);
	int partition;
	struct node newNode;
	for(int i =0; i<NumNodes; i++){
		if(Nodes[i].terminal==1){
		printf("x %d ",Nodes[i].x);
	    fprintf(fp, "%s %d ",Nodes[i].name ,Nodes[i].xCenter);
	    printf("x %d ",Nodes[i].y);
	    fprintf(fp, "%d\n",Nodes[i].yCenter);
		}
	}
	int i,p;
	int generate_x, generate_y;
	//int x_range = b.maxX - b.minX + 1 - 100;
	xGridrange[0] = b.minX;
	yGridrange[0] = b.minY;
	width_xgrid = (b.maxX - b.minX)/4;  // divide by parts // Number of cuts made by hmetis
	height_ygrid = (b.maxY - b.minY)/4; // divide by parts // 4 = Number of cuts made by hmetis
	for(i=1; i < 5; i++){
		xGridrange[i] = xGridrange[i-1] + width_xgrid;
		yGridrange[i] = yGridrange[i-1] + height_ygrid;
		printf("xgrid[%d] %d ygrid[%d] %d\n",i, xGridrange[i],i, yGridrange[i]);
	}

	for(i=NumTerminals; i < NumNodes; i++){
		p = Nodes[i].partition;
		generate_x = (rand() % width_xgrid) + xGridrange[p % 4];
		generate_y = (rand() % height_ygrid) + yGridrange[p / 4];
		if(p % 4 == 3) generate_x -= Nodes[i].width;
		if(p / 4 == 3) generate_y -= Nodes1[i].height;
		calNodecenter(i, generate_x, generate_y);
		fprintf(fp, "%s %d %d\n",Nodes[i].name ,Nodes[i].xCenter, Nodes[i].yCenter);


	}

	

}




void readPartitionFile(){
	FILE *fp;
	char buf1[100];
	int num = 0, x, y;
	fp = fopen("ibm01/ibm01_partition4.txt", "r"); 
	if (fp == NULL){
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
    }

 	for(int i =0; i < NumNodes; i++){
 		fscanf(fp, "%d", &Nodes[i].partition);
 		printf("p %d\n",Nodes[i].partition);

 	}


}




int wirelength(){
	int wirelength =0;
	int x = 0;
	int y= 0;
	for(int i =0; i < NumNets ; i++){
		int minXBound = 500000, minYBound = 500000, maxXBound = -500000, maxYBound = -500000;
		for (int j = 0; j < NetDegree[i]; ++j){
			x = Nodes[NetList[i][j]].xCenter;
			y = Nodes[NetList[i][j]].yCenter;

			if(x < minXBound) {
					minXBound = x;
				}
				if(x > maxXBound) {
					maxXBound = x;
				}
				if(y < minYBound) {
					minYBound = y;
				}
				if(y > maxYBound) {
					maxYBound = y;
			
		}
	}
	wirelength += (abs(maxXBound - minXBound)) + (abs(maxYBound - minYBound));

}
return wirelength;
printf("wirelength %d\n",wirelength);

}
// Improve ASAP

long int cellOverlap()
{
	int i, j;
	int x1_1, x1_2, y1_1, y1_2;
	int x2_1, x2_2, y2_1, y2_2;
	int xOverlap, yOverlap;
	long int overlap;
	long int totalOverlap;
	totalOverlap = 0;
	for(i = NumTerminals; i < NumNodes-1; i++){  //
		x1_1 = Nodes[i].x;
		x1_2 = Nodes[i].width + x1_1;
		y1_1 = Nodes[i].y;
		y1_2 = Nodes[i].height + y1_1;
		overlap = 0;
		for(j = i+1; j < NumNodes; j++){
			x2_1 = Nodes[j].x;
			x2_2 = Nodes[j].width + x2_1;
			y2_1 = Nodes[j].y;
			y2_2 = Nodes[j].height + y2_1;
			if(x1_1 <= x2_1){
				if(x1_2 <= x2_1) xOverlap = 0;
				else if(x2_1 < x1_2 && x1_2 <= x2_2) xOverlap = x1_2 - x2_1;
				else xOverlap = Nodes[j].width;
			} else {
				if(x2_2 <= x1_1) xOverlap = 0;
				else if(x1_1 < x2_2 && x2_2 <= x1_2) xOverlap = x2_2 - x1_1;
				else xOverlap = Nodes[i].width;
			}
			if(y1_1 <= y2_1){
				if(y1_2 <= y2_1) yOverlap = 0;
				else if(y2_1 < y1_2 && y1_2 <= y2_2) yOverlap = y1_2 - y2_1;
				else yOverlap = Nodes[j].height;
			} else {
				if(y2_2 <= y1_1) yOverlap = 0;
				else if(y1_1 < y2_2 && y2_2 <= y1_2) yOverlap = y2_2 - y1_1;
				else yOverlap = Nodes[i].height;
			}
			overlap+=xOverlap*yOverlap;
		}
 
		totalOverlap+=overlap;
	}
	return totalOverlap;
	printf("totaloverlap %ld\n", totalOverlap);
}

void overlapcal(){
	int overlaparea = 0;
	int totaloverlap = 0;
	int x_overlap = 0;
	int y_overlap = 0;
	int x1_1,x1_2,y1_2,y1_1;
	int x2_1,x2_2,y2_2, y2_1;
	for(int i = NumTerminals; i < NumNodes ; i++){
		//printf(i);
		 x1_1 = Nodes[i].x;
		 x1_2 = x1_1 + Nodes[i].width;
		 y1_1 = Nodes[i].y;
		 y1_2 = y1_1 + Nodes[i].height;
		 //overlaparea = 0;
		for (int j = i +1; j < NumNodes; j++)
		{
			//printf("y\n");

			 x2_1 = Nodes[j].x;
			 x2_2 = x2_1 + Nodes[j].width;
			 y2_1 = Nodes[j].y;
			 y2_2 = y2_1 + Nodes[j].height;
			
			if(x1_1 <= x2_1) {
				if(x1_2 <= x2_1)  {x_overlap =0;}
				else if(x2_1 < x1_2 && x1_2 <= x2_2) {x_overlap = x1_2 - x2_1;}
				else {x_overlap = Nodes[j].width;}
			}
			else{
				if(x2_2 <= x1_1) {x_overlap = 0;}
				else if(x1_1 < x2_2 && x2_2 < x1_2) {x_overlap = x2_2 - x1_1;}
				else {x_overlap = Nodes[i].width;}
			}
			if(y1_1 <= y2_1){
				if(y1_2 <= y2_1) {y_overlap=0;}
				else if (y2_1 < y1_2 && y1_2 <= y2_2) {y_overlap = y1_2 - y2_1;}
				else {y_overlap = Nodes[j].height;}
			}
			else{
				if(y2_2 <= y1_1)  {y_overlap = 0;}
				else if(y1_1 < y2_2 && y2_2 <= y1_2)  {y_overlap = y2_2 - y1_1;}
				else {y_overlap = Nodes[i].height;}
			}
			//printf("x y %d %d\n",x_overlap,y_overlap);
			overlaparea = overlaparea + x_overlap * y_overlap;

		}
		
	}

	totaloverlap = totaloverlap + overlaparea;
	printf("totaloverlap %d\n",totaloverlap);


}

void checkinputnodes(){
	for(int i= 0; i < NumNodes ; i++){
		printf("%s %d %d\n", Nodes[i].name, Nodes[i].width, Nodes[i].height);
	}
}

int costfunction(){   // change costfunction
	int cost = 0;
	long int overlaparea = cellOverlap();
	int length = wirelength();

	double alpha1 = 10;
	double alpha2 = 1;

	cost = (int) ((alpha1 * overlaparea) + (alpha2 * length));
	return cost;

}

/*int swapCost(){
	printf("swapping nodes\n");
	int randomNum1, randomNum2;
	int randomMin = 1;
	int randomMax = NumNodes - NumTerminals;
		//while((randomNum1 == 0 && randomNum2 == 0)) {
			randomNum1 = (int)(random() % (randomMax - randomMin) + randomMin);  // improve to swap only inside partitions
			randomNum2 = (int)(random() % (randomMax - randomMin) + randomMin);
			//if(randomNum1 == randomNum2) {
				//randomNum1 = randomNum2 = 0;
				//continue;
			//}
          //}
		int oldcost = costfunction();
		printf("Old cost%d\n", oldcost);	
		int temp1_x, temp1_y, temp2_x, temp2_y;
		temp1_x = Nodes[randomNum1].x;
		temp1_y = Nodes[randomNum1].y;
		temp2_x = Nodes[randomNum2].x;
		temp2_y = Nodes[randomNum2].y;
		int newCost = costfunction();
		int delta = newCost - oldcost;
		return delta;

}
*/

void swap(float temperature){
	printf("swapping nodes\n");
	printf("%f\n",temperature);

	int randomNum1, randomNum2;
	int randomMin = 1;
	int randomMax = NumNodes - NumTerminals;
		//while((randomNum1 == 0 && randomNum2 == 0)) {
			randomNum1 = (int)(random() % (randomMax - randomMin) + randomMin);  // improve to swap only inside partitions
			randomNum2 = (int)(random() % (randomMax - randomMin) + randomMin);
			//if(randomNum1 == randomNum2) {
				//randomNum1 = randomNum2 = 0;
				//continue;
			//}
          //} // simulated annealing whether to accept move or not is done here
		int acceptmove;
		int oldcost = costfunction();
		printf("Old cost%d\n", oldcost);	
		int temp1_x, temp1_y, temp2_x, temp2_y;
		temp1_x = Nodes[randomNum1].x;
		temp1_y = Nodes[randomNum1].y;
		temp2_x = Nodes[randomNum2].x;
		temp2_y = Nodes[randomNum2].y;
		calNodecenter(randomNum1, temp2_x, temp2_y);
		calNodecenter(randomNum2 ,temp1_x, temp1_y);
		int newCost = costfunction();
		printf("New cost%d\n", newCost);
		int delta = newCost - oldcost;
		double randomNumber = rand();
		double x = exp(-delta/temperature);
		if((delta < 0) || (randomNumber < x)){
			acceptmove = 1;
			printf("Swap Move Accepted\n");
		}
		else{
			acceptmove = 0;
			printf("NodeMove Rejected\n");

		}

		if(!acceptmove){
		calNodecenter(randomNum1, temp1_x, temp1_y);
		calNodecenter(randomNum2 ,temp2_x, temp2_y);
		}
		printf("a%drandonNode1 %d %d %d %d \n", randomNum1, Nodes[randomNum1].x, Nodes[randomNum1].y, temp2_x, temp2_y );

	/*
	Nodestemp.x = Nodes1.x;
	Nodestemp.y = Nodes1.y;
	Nodestemp.xCenter = Nodes1.xCenter;
	Nodestemp.ycenter = Nodes1.ycenter;

	Nodes1.x = Nodes2.x;
	Nodes1.y = Nodes2.y;
	Nodes1.xCenter =  Nodes2.xCenter;
	Nodes1.yCenter = Nodes2.yCenter;

	Nodes2.x = Nodes1.x;
	Nodes2.y = Nodes1.y;
	Nodes2.xCenter = Nodes1.xCenter;
	Nodes2.yCenter = Nodes1.yCenter;
*/


}

void NodeMove(float temperature){
		printf("Moving node\n");
		printf("%f\n",temperature);
		int block = 500;
		calNodecenter(500 ,Nodes[500].x, Nodes[500].y);

		int randomNode;
		int randomNum_x;
		int randomNum_y;
		int randomMin = 501;
		int randomMax = NumNodes - NumTerminals;
		int oldcost = costfunction();
		printf("Old cost%d\n", oldcost);

		do
		{

		randomNum_x = (int)(random() % (b.maxX - b.minX) + b.minX);  // improve to swap only inside partitions
		randomNum_y = (int)(random() % (b.maxY - b.minY) + b.minY);
		randomNode = (int)(random() % (randomMax - randomMin ) + randomMin);
		//break;
	}
	while( ((randomNum_x > b.minY) && ((randomNum_x + Nodes[randomNode].width ) < b.maxY)) && ((randomNum_y > b.minX) &&  ((Nodes[randomNode].height + randomNum_y) < b.maxX)  ) );
	int num = randomNode;
	int tempx = Nodes[randomNode].x;
	int tempy = Nodes[randomNode].y;
	int acceptmove ;
	calNodecenter(randomNode ,randomNum_x, randomNum_y);
	int newCost = costfunction();
	printf("a%d %d %d \n", randomNode, randomNum_x, randomNum_y);
	printf("%d\n",newCost);
		int delta = newCost - oldcost;
		double randomNumber = rand();        // simulated annealing whether to accept move or not is done here
		double x = exp(-delta/temperature);
		if((delta < 0) || (randomNumber < x)){
			acceptmove = 1;
			printf("NodeMove Accepted\n");
		}
		else{
			acceptmove = 0;
			printf("NodeMove Rejected\n");

		}

	if(!acceptmove){
	calNodecenter(num ,tempx, tempy);
	printf("a%d %d %d\n", num, tempx,tempy);
	}
	

}


void TimberwolfMetropolis(){
	srand((unsigned)time(NULL));
	float temperature = 4000000;
	float time = 0;
	float alpha = (float) 0.8;
	int M = 120; // change made here
	printf("Timberwolf Algorithm starts\n");
	//T_N = Metropolis(temperature, M, T_N);
	while(temperature > 0.1){
		printf("start\n");
		int oldcost = 0;
		/**** Metropolis starts here *****/
		while(M > 0){
			//oldcost = costfunction();
			//printf("Old cost %d \n", oldcost);
			/* Perturn Function starts her */
			int min =1;
			int max =3;
			int x = (int)( rand() % ((max-min)+min));
			if (x == 1)
			{
				printf("Cellswap \n");
				swap(temperature);

			}
			else if(x == 2){
				printf("CellMove \n");
				swap(temperature);		// Nodemove		
			}
			M--;

		} //perturb ends here
		printf("M value %d\n",M);
		temperature = 0.8 * temperature;
		//printf("New temperature%f\n",temperature );
		M=120;

		if(temperature < 1000000 && (temperature >= 666667)) {
			alpha = (float) 0.8;
			temperature = alpha * temperature;
			printf("This temperature taken\n");

		}
		else if((temperature < 666667) && (temperature >= 333333)) {
			alpha = (float) 0.95;
			temperature = alpha * temperature;

		}
		else if((temperature < 333333) && (temperature > 0.1)) {
			alpha = (float) 0.8;
			temperature = alpha * temperature;

		}



	}


}

void FinalPlacement(){
	FILE *fp = NULL;
	fp = fopen("Test/ibm01_output.pl", "w+"); 
	if(fp == NULL) printf("The error does not exist\n");
	
	fprintf(fp, "NumNodes :%d\n", NumNodes);
	fprintf(fp, "NumTerminals :%d\n", NumTerminals);
	int i, j;
	for(i = 0; i < NumTerminals; i++){
		fprintf(fp, "p%d %d %d\n", i+1, Nodes[i].xCenter, Nodes[i].yCenter);
	}
	for(i = NumTerminals; i < NumNodes; i++){
		fprintf(fp, "a%d %d %d\n", i-NumTerminals, Nodes[i].xCenter, Nodes[i].yCenter);
	}
	fclose(fp);
}





int main(){
	readNodesFile();
	readPlFile();
	readNetsFile();
	BoundaryCalculation();
	//int partition = 4;
	writeGraphFile();
	//initialplacement();
	GridDivision();
	readPartitionFile();
	//checkinputnodes();
	wirelength();
	//overlapcal();
	cellOverlap();
	long int cost2 = cellOverlap();
	printf("%ld\n",cost2);
	initialplacement1();
	TimberwolfMetropolis();
	FinalPlacement();
	//swap();
	//NodeMove();

	
	return 0;
}

