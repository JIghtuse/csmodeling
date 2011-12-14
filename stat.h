/* file: stat.h */
typedef struct {
	float sum;
	float sqsum;
	int count;
	int rcount;
} stat;

void print_stat (stat *var);
void init_stat (stat *var); 
void change_stat (stat *var, float val);
