/* file: stat.h */
typedef struct {
	float sum;
	float sqsum;
	unsigned int count;
} stat;

void print_stat (stat *var);
void init_stat (stat *var); 
void change_stat (stat *var, float val);
