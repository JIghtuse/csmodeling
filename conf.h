/* file: conf.h */
#define CONF "model.conf"

int N, V, vmax, kmax;
float d, lambda, D, mu, delta, Tk;

int read_conf(void);
void parse_param(char *par_name, float par_value);
