#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>
#include <libintl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <libconfig.h>

#define PACKAGE    "LS bash config"
#define VERSION    "0.0.1"

struct flags {
	int quiet;
	int names;
	int types;
	int values;
	int indexes;
	int counter;
	int unset;
	int boolstring;
	int mode;
	int error;
};

int getNumber() {
  char buf[1000];
  int test,val;
  unsigned int inp;
  fgets(buf, sizeof buf, stdin);
  test = sscanf(buf, "%u", &inp);
  val = (int) inp;
  if(val < 0) val *= -1;
  if(test > 0) return val;
  return (int) 0;
}

void printHelp() {
	printf(gettext("Obsluga plikuow konfiguracyjnych\n"));
	printf("\n");
	printf(gettext("Skladnia: ls-config [OPCJA]\n"));
	printf(gettext("Odczytuje i zapisuje dane z plikow konfiguracyjnych\n"));
	printf(gettext("w formacie libconfig9\n"));
	printf("\n");
	printf(gettext("UWAGA: niezbedne jest podanie pliku konfiguracyjnego!\n"));
	printf("\n");
	printf(gettext("Dostepne opcje:\n"));
	printf(gettext("   -f, --file=PLIK       Plik konfiguracyjny do obsluzenia\n"));
	printf("\n");
	printf(gettext("   -s, --set=SCIEZKA     Ustawienie wartosci konfiguracyjnej o podanej sciezce\n"));
	printf(gettext("   -d, --data=DANE       Wartosc do ustawienia (tylko z -s)\n"));
	printf(gettext("   -p, --type=TYP        Typ wartosci do ustawienia\n"));
	printf("\n");
	printf(gettext("   -g, --get=SCIEZKA     Pobranie danych konfiguracyjnych o podanej sciezce\n"));
	printf(gettext("   -n, --names           Wypisz nazwy zmiennych\n"));
	printf(gettext("   -t, --types           Wypisz typy zmiennych\n"));
	printf(gettext("   -v, --values          Wypisz wartosci zmiennych\n"));
	printf(gettext("   -i, --indexes         Wypisz indeksy zmiennych\n"));
	printf(gettext("   -c, --count           Wypisz ilosc elementow (tylko: array, list, group)\n"));
	printf(gettext("   -b, --bool-string     Wypisz wartosci logiczne tekstowo\n"));
	printf("\n");
	printf(gettext("   -q, --quiet           Ciche wyjscie do uzycia w skryptach\n"));
	printf(gettext("   -h, --help            Wyswietla niniejszy opis\n"));
	printf("\n");
	printf(gettext("TYP:     typ zmiennej:\n"));
	printf(gettext("         group  - grupa zmiennych\n"));
	printf(gettext("         array  - tablica zmiennych\n"));
	printf(gettext("         list   - lista zmiennych\n"));
	printf(gettext("         int    - liczba calkowita\n"));
	printf(gettext("         int64  - liczba calkowita 64 bitowa \n"));
	printf(gettext("         float  - liczba zmiennoprzecinkowa \n"));
	printf(gettext("         bool   - wartosc logiczna \n"));
	printf(gettext("         string - lancuch znakowy \n"));
	printf("\n");
	printf("(c) 2013 by LucaS web sutio - http://www.lucas.net.pl\n");
	printf("Author: Łukasz A. Grabowski\n");
   exit(0);
};

int set_config_int(config_setting_t *setting, char *dataString, struct flags optflags) {
	long bufl;
	int buf, scs;
	char *erp;	
	errno = 0;
	bufl = strtol(dataString, &erp, 0);
	if(((errno == ERANGE && (bufl == LONG_MAX || bufl == LONG_MIN)) || (errno != 0 && bufl == 0)) || (erp == dataString) || bufl > INT_MAX || bufl < INT_MIN) {
		if(optflags.quiet == 0) printf(gettext("BLAD! Bledny format danych.\n"));
  			return 12;
		};
	buf = (int)bufl;
	scs = config_setting_set_int(setting, buf);
	if(scs == CONFIG_FALSE) {
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
		return 11;
	};	
	return 0;
};

int set_config_int64(config_setting_t *setting, char *dataString, struct flags optflags) {
	long bufl;
	int scs;
	char *erp;	
	errno = 0;
	bufl = strtol(dataString, &erp, 0);
	if(((errno == ERANGE && (bufl == LONG_MAX || bufl == LONG_MIN)) || (errno != 0 && bufl == 0)) || (erp == dataString)) {
		if(optflags.quiet == 0) printf(gettext("BLAD! Bledny format danych.\n"));
			return 12;
		};
	scs = config_setting_set_int64(setting, bufl);
	if(scs == CONFIG_FALSE) {
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
		return 11;
	};	
	return 0;
};

int set_config_float(config_setting_t *setting, char *dataString, struct flags optflags) {
	double buff;
	int scs;
	char *erp;	
	errno = 0;
	buff = strtod(dataString, &erp);
   if(((errno == ERANGE && (buff == HUGE_VALF || buff == HUGE_VALL)) || (errno != 0 && buff == 0)) || (erp == dataString)) {
		if(optflags.quiet == 0) printf(gettext("BLAD! Bledny format danych.\n"));
		return 12;
	}
	scs = config_setting_set_float(setting, buff);
	if(scs == CONFIG_FALSE) {
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
		return 11;
	};	
	return 0;
};

int set_config_bool(config_setting_t *setting, char *dataString, struct flags optflags) {
	int scs, buf;
	buf = -1;
	if(!strcmp(dataString, "1") || !strcmp(dataString, "true") || !strcmp(dataString, "TRUE")) buf = 1;
	if(!strcmp(dataString, "0") || !strcmp(dataString, "false") || !strcmp(dataString, "FALSE")) buf = 0;
	if(buf < 0) {
		if(optflags.quiet == 0) printf(gettext("BLAD! Bledny format danych.\n"));
		return 12;
	}
	scs = config_setting_set_bool(setting, buf);
	if(scs == CONFIG_FALSE) {
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
		return 11;
	};	
	return 0;
};

char* path_parent(char *dataPath) {
	char *str_ptr, *last_ptr, *newpath, *dot=".";
	newpath = malloc(1);
	memset(newpath, 0, 1);
	last_ptr = malloc(1);
	memset(last_ptr, 0, 1);
	
	str_ptr = strtok(dataPath, ".");
	last_ptr = (char*)realloc(last_ptr, (strlen(str_ptr)+1)*sizeof(char));
	strcpy(last_ptr, str_ptr);

	while(str_ptr != NULL) {
		str_ptr = strtok(NULL, ".");
		if(str_ptr != NULL) {
			if(strlen(last_ptr) > 0 ) {
				newpath = (char*)realloc(newpath, (strlen(newpath)+strlen(last_ptr)+2)*sizeof(char));
				strcat(newpath, dot);
				strcat(newpath, last_ptr);
			};
			last_ptr = (char*)realloc(last_ptr, (strlen(str_ptr)+1)*sizeof(char));
			strcpy(last_ptr, str_ptr);
		} else {
			last_ptr = (char*)realloc(last_ptr, (1)*sizeof(char));
			memset(last_ptr, 0, 1);
		};
  	};
	free(dataPath);
	if(strlen(newpath) == 0) {
		free(newpath);
		newpath = NULL;
	};
	return newpath;
};

char* path_name(char *dataPath) {
	char *str_ptr, *name, *tk;
	name = malloc(1);
	tk = malloc((strlen(dataPath)+1)*sizeof(char));
	memset(name, 0, 1);
	strcpy(tk, dataPath);

	str_ptr = strtok(tk, ".");

	while(str_ptr != NULL) {
		name = (char*)realloc(name, (strlen(str_ptr)+1)*sizeof(char));
		strcpy(name, str_ptr);
		str_ptr = strtok(NULL, ".");
  	};
	free(tk);
	if(strlen(name) == 0) {
		free(name);
		name = NULL;
	};
	return name;
};

int set_config(char *configFile, char *dataPath, struct flags optflags, char *dataString, char *dataType) {
	config_t cfg;
	config_setting_t *setting, *ss;
	config_init(&cfg);
	int scs, dt, dattyp;
	char *npath;
	if(!config_read_file(&cfg, configFile)) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie mozna otworzyc pliku konfiguracyjnego.\n"));
  		return 1;
 	};
	if(dataPath == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie podano sciezki zmiennej.\n"));
  		return 4;
	};
	if(dataString == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie podano wartosci zmiennej.\n"));
  		return 9;
	};
	setting = config_lookup(&cfg, dataPath);
	if(setting == NULL) {
		npath = path_name(dataPath);
		dataPath = path_parent(dataPath);
		if(dataPath == NULL) {		
			setting = config_root_setting(&cfg);
		} else {
			setting = config_lookup(&cfg, dataPath);
		};
		if(setting == NULL) {
			config_destroy(&cfg);
			if(optflags.quiet == 0) printf(gettext("BLAD! Bledna sciezka zmiennej.\n"));
  			return 16;
		};
		dt = config_setting_type(setting);
		if(dt != CONFIG_TYPE_GROUP) {
			config_destroy(&cfg);
			if(optflags.quiet == 0) printf(gettext("BLAD! Nowa nazwe zmiennej mozna dodac tylko do grupy.\n"));
  			return 17;
		};
		if(dataType == NULL) {
			config_destroy(&cfg);
			if(optflags.quiet == 0) printf(gettext("BLAD! Nie okreslono typu zmiennej.\n"));
  			return 13;
		};
		if(!strcmp(dataType, "int")) {
			dattyp = CONFIG_TYPE_INT;
		} else if(!strcmp(dataType, "int64")) {
			dattyp = CONFIG_TYPE_INT64;
		} else if(!strcmp(dataType, "float")) {
			dattyp = CONFIG_TYPE_FLOAT;
		} else if(!strcmp(dataType, "string")) {
			dattyp = CONFIG_TYPE_STRING;
		} else if(!strcmp(dataType, "bool")) {
			dattyp = CONFIG_TYPE_BOOL;
		} else if(!strcmp(dataType, "array")) {
			dattyp = CONFIG_TYPE_ARRAY;
		} else if(!strcmp(dataType, "list")) {
			dattyp = CONFIG_TYPE_LIST;
		} else if(!strcmp(dataType, "group")) {
			dattyp = CONFIG_TYPE_GROUP;
		} else {
			config_destroy(&cfg);
			if(optflags.quiet == 0) printf(gettext("BLAD! Niedozwolony typ zmiennej.\n"));
  			return 14;
		};
		ss = config_setting_add(setting, npath, dattyp);
		if(ss == NULL) {
			config_destroy(&cfg);
			if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  			return 11;
		};
		scs = 0;
		switch(dattyp) {
			case CONFIG_TYPE_INT:
				scs = set_config_int(ss, dataString, optflags);
				break;
			case CONFIG_TYPE_INT64:
				scs = set_config_int64(ss, dataString, optflags);
				break;
			case CONFIG_TYPE_FLOAT:
				scs = set_config_float(ss, dataString, optflags);
				break;
			case CONFIG_TYPE_STRING:
				scs = config_setting_set_string(ss, dataString);
				if(scs == CONFIG_FALSE) {
					if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  					scs = 11;
				} else scs = 0;
				break;
			case CONFIG_TYPE_BOOL:
				scs = set_config_bool(ss, dataString, optflags);
				break;
		};
		if(scs > 0) {
			config_destroy(&cfg);
			return scs;
      };
	} else {
		dt = config_setting_type(setting);
		switch(dt) {
			case CONFIG_TYPE_INT:
				if(dataType != NULL && strcmp(dataType, "int")) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Niezgodny typ zmiennej.\n"));
  					return 10;
				};	
				scs = set_config_int(setting, dataString, optflags);
				if(scs > 0) {
					config_destroy(&cfg);
  					return scs;
				};	
				break;
			case CONFIG_TYPE_INT64:
				if(dataType != NULL && strcmp(dataType, "int64")) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Niezgodny typ zmiennej.\n"));
  					return 10;
				};	
				scs = set_config_int64(setting, dataString, optflags);
				if(scs > 0) {
					config_destroy(&cfg);
  					return scs;
				};	
				break;
			case CONFIG_TYPE_FLOAT:
				if(dataType != NULL && strcmp(dataType, "float")) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Niezgodny typ zmiennej.\n"));
  					return 10;
				};	
				scs = set_config_float(setting, dataString, optflags);
				if(scs > 0) {
					config_destroy(&cfg);
  					return scs;
				};	
				break;
			case CONFIG_TYPE_STRING:
				if(dataType != NULL && strcmp(dataType, "string")) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Niezgodny typ zmiennej.\n"));
  					return 10;
				};	
				scs = config_setting_set_string(setting, dataString);
				if(scs == CONFIG_FALSE) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  					return 11;
				};	
				break;
			case CONFIG_TYPE_BOOL:
				if(dataType != NULL && strcmp(dataType, "bool")) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Niezgodny typ zmiennej.\n"));
  					return 10;
				};
				scs = set_config_bool(setting, dataString, optflags);
				if(scs > 0) {
					config_destroy(&cfg);
  					return scs;
				};	
				break;
			case CONFIG_TYPE_ARRAY:
				if(config_setting_length(setting) == 0) {
					if(dataType == NULL) {	
						config_destroy(&cfg);
						if(optflags.quiet == 0) printf(gettext("BLAD! Nie okreslono typu zmiennej.\n"));
  						return 13;
					};
					if(!strcmp(dataType, "int")) {
						dattyp = CONFIG_TYPE_INT;
					} else if(!strcmp(dataType, "int64")) {
						dattyp = CONFIG_TYPE_INT64;
					} else if(!strcmp(dataType, "float")) {
						dattyp = CONFIG_TYPE_FLOAT;
					} else if(!strcmp(dataType, "string")) {
						dattyp = CONFIG_TYPE_STRING;
					} else if(!strcmp(dataType, "bool")) {
						dattyp = CONFIG_TYPE_BOOL;
					} else {
						config_destroy(&cfg);
						if(optflags.quiet == 0) printf(gettext("BLAD! Niedozwolony typ zmiennej.\n"));
  						return 14;
					};
					ss = config_setting_add(setting, NULL, dattyp);
					if(ss == NULL) {
						config_destroy(&cfg);
						if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  						return 11;
					};
					switch(dattyp) {
						case CONFIG_TYPE_INT:
							scs = set_config_int(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
  								return scs;
							};	
							break;
						case CONFIG_TYPE_INT64:
							scs = set_config_int64(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
  								return scs;
							};	
							break;
						case CONFIG_TYPE_FLOAT:
							scs = set_config_float(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
  								return scs;
							};	
							break;
						case CONFIG_TYPE_STRING:
							scs = config_setting_set_string(ss, dataString);
							if(scs == CONFIG_FALSE) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  								return 11;
							};
							break;
						case CONFIG_TYPE_BOOL:
							scs = set_config_bool(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
  								return scs;
							};	
							break;
					};
				} else {
					dattyp = config_setting_type(config_setting_get_elem(setting, 0));
					switch(dattyp) {
						case CONFIG_TYPE_INT:
							if(dataType != NULL && strcmp(dataType, "int")) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Niezgodny typ zmiennej.\n"));
  								return 10;
							};
							ss = config_setting_add(setting, NULL, dattyp);
							if(ss == NULL) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  								return 11;
							};
							scs = set_config_int(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
								return scs;
							};
							break;
						case CONFIG_TYPE_INT64:
							if(dataType != NULL && strcmp(dataType, "int64")) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Niezgodny typ zmiennej.\n"));
  								return 10;
							};
							ss = config_setting_add(setting, NULL, dattyp);
							if(ss == NULL) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  								return 11;
							};
							scs = set_config_int64(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
								return scs;
							};
							break;
						case CONFIG_TYPE_FLOAT:
							if(dataType != NULL && strcmp(dataType, "float")) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Niezgodny typ zmiennej.\n"));
  								return 10;
							};
							ss = config_setting_add(setting, NULL, dattyp);
							if(ss == NULL) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  								return 11;
							};
							scs = set_config_float(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
								return scs;
							};
							break;
						case CONFIG_TYPE_STRING:
							if(dataType != NULL && strcmp(dataType, "string")) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Niezgodny typ zmiennej.\n"));
  								return 10;
							};
							ss = config_setting_add(setting, NULL, dattyp);
							if(ss == NULL) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  								return 11;
							};
							scs = config_setting_set_string(ss, dataString);
							if(scs == CONFIG_FALSE) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  								return 11;
							};
							break;
						case CONFIG_TYPE_BOOL:
							if(dataType != NULL && strcmp(dataType, "bool")) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Niezgodny typ zmiennej.\n"));
  								return 10;
							};
							ss = config_setting_add(setting, NULL, dattyp);
							if(ss == NULL) {
								config_destroy(&cfg);
								if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  								return 11;
							};
							scs = set_config_bool(ss, dataString, optflags);
							if(scs > 0) {
								config_destroy(&cfg);
								return scs;
							};
							break;
					};
				};
				break;
			case CONFIG_TYPE_LIST:
				if(dataType == NULL) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Nie okreslono typu zmiennej.\n"));
  					return 13;
				};
				if(!strcmp(dataType, "int")) {
					dattyp = CONFIG_TYPE_INT;
				} else if(!strcmp(dataType, "int64")) {
					dattyp = CONFIG_TYPE_INT64;
				} else if(!strcmp(dataType, "float")) {
					dattyp = CONFIG_TYPE_FLOAT;
				} else if(!strcmp(dataType, "string")) {
					dattyp = CONFIG_TYPE_STRING;
				} else if(!strcmp(dataType, "bool")) {
					dattyp = CONFIG_TYPE_BOOL;
				} else if(!strcmp(dataType, "array")) {
					dattyp = CONFIG_TYPE_ARRAY;
				} else if(!strcmp(dataType, "list")) {
					dattyp = CONFIG_TYPE_LIST;
				} else if(!strcmp(dataType, "group")) {
					dattyp = CONFIG_TYPE_GROUP;
				} else {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Niedozwolony typ zmiennej.\n"));
  					return 14;
				};
				ss = config_setting_add(setting, NULL, dattyp);
				if(ss == NULL) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  					return 11;
				};
				scs = 0;
				switch(dattyp) {
					case CONFIG_TYPE_INT:
						scs = set_config_int(ss, dataString, optflags);
						break;
					case CONFIG_TYPE_INT64:
						scs = set_config_int64(ss, dataString, optflags);
						break;
					case CONFIG_TYPE_FLOAT:
						scs = set_config_int64(ss, dataString, optflags);
						break;
					case CONFIG_TYPE_STRING:
						scs = config_setting_set_string(ss, dataString);
						if(scs == CONFIG_FALSE) {
							config_destroy(&cfg);
							if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  							return 11;
						};
						scs = 0;
						break;
					case CONFIG_TYPE_BOOL:
						scs = set_config_int64(ss, dataString, optflags);
						break;
				};
				if(scs > 0) {
					config_destroy(&cfg);
  					return scs;
				};
				if(optflags.quiet == 0) {
					printf(gettext("Dodano pozycje: %d\n"), config_setting_index(ss));
				} else {
					printf("%d", config_setting_index(ss));
				};
				break;
			case CONFIG_TYPE_GROUP:
				if(dataType == NULL) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Nie okreslono typu zmiennej.\n"));
  					return 13;
				};
				if(strlen(dataString) < 1) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Bledna nazwa zmiennej konfiguracyjnej.\n"));
  					return 15;
				};
				if(!strcmp(dataType, "int")) {
					dattyp = CONFIG_TYPE_INT;
				} else if(!strcmp(dataType, "int64")) {
					dattyp = CONFIG_TYPE_INT64;
				} else if(!strcmp(dataType, "float")) {
					dattyp = CONFIG_TYPE_FLOAT;
				} else if(!strcmp(dataType, "string")) {
					dattyp = CONFIG_TYPE_STRING;
				} else if(!strcmp(dataType, "bool")) {
					dattyp = CONFIG_TYPE_BOOL;
				} else if(!strcmp(dataType, "array")) {
					dattyp = CONFIG_TYPE_ARRAY;
				} else if(!strcmp(dataType, "list")) {
					dattyp = CONFIG_TYPE_LIST;
				} else if(!strcmp(dataType, "group")) {
					dattyp = CONFIG_TYPE_GROUP;
				} else {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Niedozwolony typ zmiennej.\n"));
  					return 14;
				};
				ss = config_setting_add(setting, dataString, dattyp);
				if(ss == NULL) {
					config_destroy(&cfg);
					if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie ustawic zmiennej.\n"));
  					return 11;
				};
				if(optflags.quiet == 0) {
					printf(gettext("Dodano pozycje: %d\n"), config_setting_index(ss));
				} else {
					printf("%d", config_setting_index(ss));
				};
				break;
		};
	}

	scs = config_write_file(&cfg, configFile);
	if(scs == CONFIG_FALSE) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Zapisu pliku konfiguracyjnego.\n"));
  		return 8;
 	};
 	config_destroy(&cfg);
	return 0;
};

int unset_config(char *configFile, char *dataPath, struct flags optflags) {
	config_t cfg;
	config_setting_t *setting, *par;
	config_init(&cfg);
	int idx, scs;
	if(!config_read_file(&cfg, configFile)) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie mozna otworzyc pliku konfiguracyjnego.\n"));
  		return 1;
 	};
	if(dataPath == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie podano sciezki zmiennej.\n"));
  		return 4;
	};
	setting = config_lookup(&cfg, dataPath);
	if(setting == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie odnaleziono poszukiwanej konfiguracji.\n"));
  		return 3;
 	};
	idx = config_setting_index(setting);
	if(idx < 0) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie mozna usunac glownego elementu.\n"));
  		return 5;
 	};
	par = config_setting_parent(setting);
	if(par == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie mozna znalezc elementu nadrzednego.\n"));
  		return 6;
 	};
	scs = config_setting_remove_elem(par, idx);
	if(scs == CONFIG_FALSE) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie udalo sie usunac elementu.\n"));
  		return 7;
 	};
	scs = config_write_file(&cfg, configFile);
	if(scs == CONFIG_FALSE) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Zapisu pliku konfiguracyjnego.\n"));
  		return 8;
 	};
 	config_destroy(&cfg);
	return 0;
};

int read_config(char *configFile, char *dataPath, struct flags optflags) {
	config_t cfg;
	config_setting_t *setting, *ss;
	config_init(&cfg);
	int comaset, varindex, varcounter;
	unsigned int maxel, i;
	char buffer[256];
	const char *cbuffer;
	const char *coma=";";
	int ibuffer, ssize;
	char *dataName, *dataTypeName, *dataValueString;
	int dataType, st;
	dataValueString = malloc(1);
	dataTypeName = malloc(1);
	memset(dataValueString, 0, 1);
	memset(dataTypeName, 0, 1);
	varindex = 0;
	varcounter = 0;
	if(!config_read_file(&cfg, configFile)) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie mozna otworzyc pliku konfiguracyjnego.\n"));
  		return 1;
 	};
	if(dataPath == NULL) {
		setting = config_root_setting(&cfg);
	} else {
		setting = config_lookup(&cfg, dataPath);
	};
	if(setting == NULL) {
  		config_destroy(&cfg);
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie odnaleziono poszukiwanej konfiguracji.\n"));
  		return 3;
 	};
	dataName = config_setting_name(setting);
	if(dataName == NULL) dataName = "NULL";
	dataType = config_setting_type(setting);	
	switch(dataType) {
		case CONFIG_TYPE_INT:
			dataTypeName = (char*)realloc(dataTypeName, 4*sizeof(char));
			strcpy(dataTypeName, "int");
			sprintf(buffer, "%d", config_setting_get_int(setting));
			dataValueString = (char*)realloc(dataValueString, (strlen(buffer)+1)*sizeof(char));
			strcpy(dataValueString, buffer);
			break;
		case CONFIG_TYPE_INT64:
			dataTypeName = (char*)realloc(dataTypeName, 6*sizeof(char));
			strcpy(dataTypeName, "int64");
			sprintf(buffer, "%lld", config_setting_get_int64(setting));
			dataValueString = (char*)realloc(dataValueString, (strlen(buffer)+1)*sizeof(char));
			strcpy(dataValueString, buffer);
			break;
		case CONFIG_TYPE_FLOAT:
			dataTypeName = (char*)realloc(dataTypeName, 9*sizeof(char));
			strcpy(dataTypeName, "float");
			sprintf(buffer, "%f", config_setting_get_float(setting));
			dataValueString = (char*)realloc(dataValueString, (strlen(buffer)+1)*sizeof(char));
			strcpy(dataValueString, buffer);
			break;
		case CONFIG_TYPE_STRING:
			dataTypeName = (char*)realloc(dataTypeName, 7*sizeof(char));
			strcpy(dataTypeName, "string");
			cbuffer = config_setting_get_string(setting);
			dataValueString = (char*)realloc(dataValueString, (strlen(cbuffer)+1)*sizeof(char));
			strcpy(dataValueString, cbuffer);
			break;
		case CONFIG_TYPE_BOOL:
			dataTypeName = (char*)realloc(dataTypeName, 5*sizeof(char));
			strcpy(dataTypeName, "bool");
			if(optflags.boolstring == 1) {
				ibuffer = config_setting_get_bool(setting); 
				if(ibuffer == CONFIG_TRUE) {
					dataValueString = (char*)realloc(dataValueString, 5*sizeof(char));
					strcpy(dataValueString, "true");
				} else {
					dataValueString = (char*)realloc(dataValueString, 6*sizeof(char));
					strcpy(dataValueString, "false");
				}
			} else {
					sprintf(buffer, "%d", config_setting_get_bool(setting));
					dataValueString = (char*)realloc(dataValueString, (strlen(buffer)+1)*sizeof(char));
					strcpy(dataValueString, buffer);
			};
			break;
		case CONFIG_TYPE_ARRAY:
			dataTypeName = (char*)realloc(dataTypeName, 6*sizeof(char));
			strcpy(dataTypeName, "array");
			maxel = (unsigned int)config_setting_length(setting);
			comaset = 0;
			for(i = 0; i < maxel; i++) {
				ss = config_setting_get_elem(setting, i);
				if(ss != NULL) {
					st = config_setting_type(ss);
					switch(st) {
						case CONFIG_TYPE_INT:
							sprintf(buffer, "%d", config_setting_get_int(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_INT64:
							sprintf(buffer, "%lld", config_setting_get_int64(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_FLOAT:
							sprintf(buffer, "%f", config_setting_get_float(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_STRING:
							ssize = (int)strlen(config_setting_get_string(ss));

							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+ssize+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, config_setting_get_string(ss));
						break;
						case CONFIG_TYPE_BOOL:
							if(optflags.boolstring == 1) {
								ibuffer = config_setting_get_bool(ss); 
								if(ibuffer == CONFIG_TRUE) {
									dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+4+2)*sizeof(char));
									if(comaset == 1) strcat(dataValueString, coma);
									strcat(dataValueString, "true");
								} else {
									dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+5+2)*sizeof(char));
									if(comaset == 1) strcat(dataValueString, coma);
									strcat(dataValueString, "false");
								}
							} else {
								sprintf(buffer, "%d", config_setting_get_bool(ss));
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, buffer);
							};
						break;
						case CONFIG_TYPE_ARRAY:
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+7)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "ARRAY");
						break;
						case CONFIG_TYPE_LIST:
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+6)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "LIST");
						break;
						case CONFIG_TYPE_GROUP:
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+7)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "GROUP");
						break;
					};	
					comaset = 1;
				};
			};
			break;
		case CONFIG_TYPE_LIST:
			dataTypeName = (char*)realloc(dataTypeName, 5*sizeof(char));
			strcpy(dataTypeName, "list");
			maxel = (unsigned int)config_setting_length(setting);
			comaset = 0;
			for(i = 0; i < maxel; i++) {
				ss = config_setting_get_elem(setting, i);
				if(ss != NULL) {
					st = config_setting_type(ss);
					switch(st) {
						case CONFIG_TYPE_INT:
							sprintf(buffer, "%d", config_setting_get_int(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_INT64:
							sprintf(buffer, "%lld", config_setting_get_int64(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_FLOAT:
							sprintf(buffer, "%f", config_setting_get_float(ss));
							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, buffer);
						break;
						case CONFIG_TYPE_STRING:
							ssize = (int)strlen(config_setting_get_string(ss));

							dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+ssize+2)*sizeof(char));
							if(comaset == 1) strcat(dataValueString, coma);
							strcat(dataValueString, config_setting_get_string(ss));
						break;
						case CONFIG_TYPE_BOOL:
							if(optflags.boolstring == 1) {
								ibuffer = config_setting_get_bool(ss); 
								if(ibuffer == CONFIG_TRUE) {
									dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+4+2)*sizeof(char));
									if(comaset == 1) strcat(dataValueString, coma);
									strcat(dataValueString, "true");
								} else {
									dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+5+2)*sizeof(char));
									if(comaset == 1) strcat(dataValueString, coma);
									strcat(dataValueString, "false");
								}
							} else {
								sprintf(buffer, "%d", config_setting_get_bool(ss));
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+strlen(buffer)+2)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, buffer);
							};
						break;
						case CONFIG_TYPE_ARRAY:
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+7)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "ARRAY");
						break;
						case CONFIG_TYPE_LIST:
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+6)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "LIST");
						break;
						case CONFIG_TYPE_GROUP:
								dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+7)*sizeof(char));
								if(comaset == 1) strcat(dataValueString, coma);
								strcat(dataValueString, "GROUP");
						break;
					};	
					comaset = 1;
				};
			};
			break;
		case CONFIG_TYPE_GROUP:
			dataTypeName = (char*)realloc(dataTypeName, 6*sizeof(char));
			strcpy(dataTypeName, "group");
			maxel = (unsigned int)config_setting_length(setting);
			comaset = 0;
			for(i = 0; i < maxel; i++) {
				ss = config_setting_get_elem(setting, i);
				if(ss != NULL) {
					ssize = (int)strlen(config_setting_name(ss));
					dataValueString = (char*)realloc(dataValueString, (strlen(dataValueString)+ssize+2)*sizeof(char));
					if(comaset == 1) strcat(dataValueString, coma);
					strcat(dataValueString, config_setting_name(ss));
					comaset = 1;
				};
			};
			break;
	};

	varindex = config_setting_index(setting);
	varcounter = config_setting_length(setting);

	if(optflags.names == 1 && optflags.quiet == 0) printf(gettext("Nazwa zmiennej:   %s\n"), dataName);
	if(optflags.names == 1 && optflags.quiet == 1) printf("%s", dataName);
	if((optflags.types == 1 && optflags.quiet == 1) && optflags.names == 1) printf(":");
	if(optflags.types == 1 && optflags.quiet == 0) printf(gettext("Typ zmiennej:     %s\n"), dataTypeName);
	if(optflags.types == 1 && optflags.quiet == 1) printf("%s", dataTypeName);
	if((optflags.values == 1 && optflags.quiet == 1) && (optflags.names == 1 || optflags.types == 1)) printf(":");
	if(optflags.values == 1 && optflags.quiet == 0) printf(gettext("Wartosc zmiennej: %s\n"), dataValueString);
	if(optflags.values == 1 && optflags.quiet == 1) printf("%s", dataValueString);
	if((optflags.indexes == 1 && optflags.quiet == 1) && (optflags.names == 1 || optflags.types == 1 || optflags.values == 1)) printf(":");
	if(optflags.indexes == 1 && optflags.quiet == 0) printf(gettext("Index zmiennej:   %d\n"), varindex);
	if(optflags.indexes == 1 && optflags.quiet == 1) printf("%d", varindex);
	if((optflags.counter == 1 && optflags.quiet == 1) && (optflags.names == 1 || optflags.types == 1 || optflags.values == 1 || optflags.indexes == 1)) printf(":");
	if(optflags.counter == 1 && optflags.quiet == 0) printf(gettext("Ilosc zmiennych:  %d\n"), varcounter);
	if(optflags.counter == 1 && optflags.quiet == 1) printf("%d", varcounter);
	if(optflags.quiet == 1) printf("\n");
	
	config_destroy(&cfg);
	return 0;
}

int main(int argc, char **argv) {
	setlocale(LC_ALL, "");
	bindtextdomain("lslib-config", "/usr/share/locale");
	textdomain("lslib-config");

   int opt,test;
	int fd;
	char *sinp, *dataPath=NULL, *dataString=NULL, *dataType=NULL;
	char *configFile=NULL;
   struct flags optflags = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	sinp = malloc(sizeof(char) * 256);

	struct option long_options[] = {
		/* These options set a flag. */
		{"quiet",   no_argument, &optflags.quiet, 1},
		{"names",   no_argument, &optflags.names, 1},
		{"types",   no_argument, &optflags.types, 1},
		{"values",   no_argument, &optflags.values, 1},
		{"indexes",   no_argument, &optflags.indexes, 1},
		{"count",   no_argument, &optflags.counter, 1},
		{"unset",   no_argument, &optflags.unset, 1},
		{"bool-string",   no_argument, &optflags.boolstring, 1},
		/* These options don't set a flag.
		 We distinguish them by their indices. */
		{"help", no_argument, 0, 'h'},
		{"set",  required_argument, 0, 's'},
		{"get",  optional_argument, 0, 'g'},
		{"data", required_argument, 0, 'd'},
		{"type", required_argument, 0, 'p'},
		{"file", required_argument, 0, 'f'},
		{0, 0, 0, 0}
	};

	while(1) {
		int option_index = 0;
		opt = getopt_long (argc, argv, "qntvicubs:g:d:p:hf:", long_options, &option_index);
		
		if(opt == -1) break;

		switch (opt) {
			case 0:
				/* If this option set a flag, do nothing else now. */
				if(long_options[option_index].flag != 0) break;
				if(strcmp(long_options[option_index].name, "set") == 0 && optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataPath = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataPath, sinp);
					};
					optflags.mode = 1;
				};
				if(strcmp(long_options[option_index].name, "get") == 0 && optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataPath = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataPath, sinp);
					}; 
					optflags.mode = 0;
				};
				if(strcmp(long_options[option_index].name, "data") == 0 && optarg) {
					test = sscanf(optarg, "%[^\n]s", sinp);
					if(test > 0) {
						dataString = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataString, sinp);
					}; 
				};
				if(strcmp(long_options[option_index].name, "type") == 0 && optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataType = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataType, sinp);
					}; 
				};
				if(strcmp(long_options[option_index].name, "file") == 0 && optarg) {
					test = sscanf(optarg, "%[^\n]s", sinp);
					if(test > 0) {
						configFile = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(configFile, sinp);
					}; 
				};
				break;
			case 'q':
				optflags.quiet = 1;
				break;
			case 'n':
				optflags.names = 1;
				break;
			case 't':
				optflags.types = 1;
				break;
			case 'v':
				optflags.values = 1;
				break;
			case 'i':
				optflags.indexes = 1;
				break;
			case 'c':
				optflags.counter = 1;
				break;
			case 'u':
				optflags.unset = 1;
				break;
			case 'b':
				optflags.boolstring = 1;
				break;
			case 's':
				if(optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataPath = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataPath, sinp);
					}; 
					optflags.mode = 1;
				};
				break;
			case 'g':
				if(optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataPath = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataPath, sinp);
					};
				}; 
				optflags.mode = 0;
				break;
			case 'd':
				if(optarg) {
					test = sscanf(optarg, "%[^\n]s", sinp);
					if(test > 0) {
						dataString = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataString, sinp);
					};
				}; 
				break;
			case 'p':
				if(optarg) {
					test = sscanf(optarg, "%s", sinp);
					if(test > 0) {
						dataType = (char*)malloc((strlen(sinp)+1)*sizeof(char));
						strcpy(dataType, sinp);
					};
				}; 
				break;
			case 'h':
				free(sinp);
				printHelp();
				break;
			case 'f':
				test = sscanf(optarg, "%[^\n]s", sinp);
				if(test > 0) {
					configFile = (char*)malloc((strlen(sinp)+1)*sizeof(char));
					strcpy(configFile, sinp);
				}; 
				break;
			case '?':
				break;
			default:
				break;
		}
	};

	if(optflags.mode == 0 && access(configFile, R_OK) < 0) optflags.error = 1;
	if(optflags.mode == 1) {
		fd = open(configFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if(fd < 0) {
			optflags.error = 1;
 		};
		close(fd);
   };
	if(optflags.error > 0) {
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie mozna otworzyc pliku konfiguracyjnego.\n"));
		free(sinp);
		free(configFile);
		exit(1);
   };
   if(optflags.mode == 1 && dataPath == NULL) {
		if(optflags.quiet == 0) printf(gettext("BLAD! Nie okreslono sciezki zmiennej.\n"));
		free(sinp);
		free(configFile);
		exit(1);
   };
	if(optflags.names == 0 && optflags.types == 0 && optflags.values == 0 && optflags.indexes == 0 && optflags.counter == 0) {
		optflags.names = 1;
		optflags.types = 1;
		optflags.values = 1;
	};

	int excode;
	excode = 0;

	if(optflags.mode == 0) excode = read_config(configFile, dataPath, optflags);
	if(optflags.mode == 1 && optflags.unset == 1) excode = unset_config(configFile, dataPath, optflags);
	if(optflags.mode == 1 && optflags.unset == 0) excode = set_config(configFile, dataPath, optflags, dataString, dataType);

	free(sinp);
	free(configFile);
	exit(excode);
}

