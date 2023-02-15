void insert_value(double arr[], double val, uint8_t cur_size, uint8_t max_size)
{
    if (cur_size < max_size)
    {
        if(0==cur_size)
        {
            arr[0] = val;
        } 
        else 
        {
            int insert_idx = 0;
            // find index where val is greater than equal to value at the index of array but less than value at the next index
            for(;(insert_idx < cur_size) && (val < arr[insert_idx]); ++insert_idx);
            for(int i=cur_size-1; i >= insert_idx; i--)
                arr[i+1] = arr[i];
            arr[insert_idx] = val;
        }
    }
}

double get_median(double arr[], uint8_t size){
    if ((size & 1) > 0)
        return arr[(size - 1) / 2];
    else
        return (arr[size / 2] + arr[size / 2 - 1]) / 2.0;
}