/* file: conf.h */
#define CONF "model.conf"

unsigned int N, Nb, V, Vb, vmax, kmax, ql;
float d, lambda, D, mu, delta, Tk;

int read_conf(void);
void parse_param(char *par_name, float par_value);
