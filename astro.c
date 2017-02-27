#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void greska(char *poruka){
	printf("%s\n",poruka);
	exit(1);
}

double* placks_function_wavelength(double temperature){
	double placks_constant = 6.62606957e-34;
	double speed_of_light = 299792458;
	double boltzmann_constant = 1.3806488e-23;
	double e = 2.7182818284590452353602875;

	int i;
	double wavelenghth;

	double *spectrum = malloc(sizeof(double)*2501);
	if(spectrum == NULL)
		greska("Greska pri alociranju memorije.");

	for(i=1;i<2501;i++){
		wavelenghth = i * 0.000000001;
		spectrum[i-1] = 2*placks_constant*pow(speed_of_light,2)/pow(wavelenghth,5)/(pow(e,(placks_constant/boltzmann_constant)*speed_of_light/(temperature*wavelenghth))-1);
	}
	return spectrum;
}

double* draw(double *spectrum,char *name){

	FILE *f;
	f = fopen(name,"w");
	if(f==NULL)
		greska("Greska pri otvaranju datoteke.");

	int i;
	for(i=0;i<2501;i++){
		fprintf(f,"%i %.50lf\n",i+1,spectrum[i]/1e13);
	}
	fclose(f);
	return spectrum;
}

void gaussian_function(float wavelenghth,int power, double *spectrum){
	double e = 2.7182818284590452353602875;
	double pi = 3.14159265359;
	float power2;
	power2 = power;
	int i;

	printf("%f %i\n",wavelenghth,power );
	for(i=0;i<2501;i++){
		spectrum[i] += (1/(1/(power2 * sqrt(2*pi))) * pow( e, (-pow(i-wavelenghth/10,2)/(10000*1/(power2 * power2)))))*1e10;
	}
}

double* add_emmision_spectrum(char *elements_s, double *spectrum){
	typedef struct _element{
		char name[3];
		float percentage;
	}element;

	element elements[100];
	float sum=0;
	char tmp_name[10];
	char tmp_percentage[10];
	float tmp_percentage_f;
	int i=0;
	int byte_moved = 0;

	for(;;){
		if((int)(strlen(elements_s) - byte_moved) <= 0)
			break;
		else if(sscanf(elements_s + byte_moved,"%s%s",tmp_name,tmp_percentage)==1)
			greska("Los unos elemenata.");
		else if(sscanf(elements_s + byte_moved,"%s%s",tmp_name,tmp_percentage)==0){
			if(i==0)
				greska("Niste uneli ni jedan element.");
			break;
		} else{
			sscanf(elements_s + byte_moved,"%s%s",tmp_name,tmp_percentage);
			printf("(%s),(%s)\n",tmp_name,tmp_percentage );
			tmp_percentage_f = atof(tmp_percentage);
			sum+=tmp_percentage_f;
			if(sum>100)
				greska("Suma procenata elemenata je veca od 100.");
			int len;
			len = strlen(tmp_name);
			if(len < 1 || len > 2)
				greska("Pogresna duzina simbola.");

			strcpy(elements[i].name,tmp_name);
			elements[i].percentage = tmp_percentage_f;

			i++;
			byte_moved = byte_moved + len + strlen(tmp_percentage) + 2;
		}
	}

	int n = i;
	for(i=0;i<n;i++){
		FILE *f;
		char file_name[17];
		strcpy(file_name,"emission/");
		strcat(file_name,elements[i].name);
		strcat(file_name,".txt");
//		printf("%s\n",file_name);
		f = fopen(file_name,"r");
		if(f==0)
			greska("Pogresan naziv elemnta.");

		int j;
		float wavelenghth;
		int power;
		while(fscanf(f,"%f%i",&wavelenghth,&power)==2){
			gaussian_function(wavelenghth,power,spectrum);
		}

		fclose(f);
	}
	return spectrum;
}

double* draw_dopler(int speed, double *spectrum, char *name){
	int speed_of_light = 299792458;

		FILE *f;

		f = fopen(name,"w");
		if(f==NULL)
			greska("Greska pri otvaranju datoteke.");

		int i;
		for(i=0;i<2501;i++){
			fprintf(f,"%i %.50lf\n",(int)(i+1+((double)speed/speed_of_light)*(i+1)),spectrum[i]/1e13);
		}
		fclose(f);
		return spectrum;
}

void napravi_gp(int temperatura,int brzina){
	FILE *f,*g;
	char ch;
	f = fopen("settings.txt","r");
	g = fopen("output.gp","w");
	if(f==NULL || g==NULL)
		greska("Greska pri otvaranju datoteke. tu");

	while( ( ch = fgetc(f) ) != EOF )
      fputc(ch, g);

		/* 2500 promeni za doplera */
	fprintf(g,"\nplot [0:2500] [:] \"graph.dat\" using 1:2 smooth csplines title \"Plankova kriva na 5250 stepeni\",\"graph2.dat\" title \"Spektar objekta (bez radijalne brzine)\" with linespoints ls 1 lc rgb \'#00ff00\', \"graph3.dat\" title \"Spektar objekta\"  with linespoints ls 1 lw 1.3\npause -1");


	fclose(f);
	fclose(g);
}

int main(){
	int temperatura,brzina;
	char elementi[100];

		printf("Unesite temperaturu objekta: ");
		scanf("%i",&temperatura);
		getchar();
		printf("\nUnesite elemente objekta, i procenat ukupne mase koji zauzima taj element.\nPrimer: H 70 He 30\n");
		fgets(elementi,100,stdin);
		printf("\nUnesite radijalnu brzinu (km/s): \n");
		scanf("%i",&brzina);

	printf("%s\n",elementi);
	double *spectrum = placks_function_wavelength(temperatura);
	draw(spectrum,"graph.dat");
	spectrum = add_emmision_spectrum(elementi,spectrum);
	draw(spectrum,"graph2.dat");
	draw_dopler(brzina,spectrum,"graph3.dat");
	napravi_gp(temperatura,brzina);
	return 0;
}
