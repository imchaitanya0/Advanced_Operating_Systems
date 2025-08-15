#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<new>
#include<string>
#include<vector>


#define BUFFER_SIZE 8192

//buffer contents reversing function...
void reverse_content_in_file(char* buffer, int size)
{
    int l=0,r=size-1;

    while(l<r)
    {
        char t=buffer[l];
        buffer[l]=buffer[r];
        buffer[r]=t;

        l++;
        r--;
    }

}

//block wise file reversal flag verification function...
bool blockwise_verification(int new_fd,int old_fd,long blk_size)
{
    if (blk_size <= 0)
    {
        fprintf(stderr,"\nBlock size cannot be less than or equal to zero\n");
        return false;
    }
    //creating empty buffers and allocating memory carefully using the exception handling...
    char *new_file_buff=nullptr,*old_file_buff=nullptr;
    bool allocation = false;
    try{
        new_file_buff = new char[blk_size];
        old_file_buff =new char[blk_size];
    }
    catch (const std::bad_alloc& e){
        fprintf(stderr, " \n Error Occurred : Memory allocation for block size you have mentioned is %ld, which is larger than the avilable RAM, so try giving appropriate block size with in limits.\n",blk_size);
        if(new_file_buff)
            delete[] new_file_buff;
        return false;
    }

    long long total_data_read=0;
    while(1)
    {
        //reading data from q1 generated output file...
        ssize_t new_file_read=read(new_fd,new_file_buff,blk_size);
        //reading data from actual input file that id given to the question-1...
        ssize_t old_file_read=read(old_fd,old_file_buff,blk_size);

        //if reading from the files fails return error to the user...
        if(new_file_read == -1  || old_file_read == -1)
        {
            perror("\nreading the file contents from the originals into buffer failed.\n");
            delete[] new_file_buff;
            delete[] old_file_buff;
            return false;

        }
        //checking whether the contents read by both the files are of equal size or not....
        if(new_file_read != old_file_read)
        {
            fprintf(stderr,"\nThe data size of contents read from the original files is different so we cant match them.\n");
            delete[] new_file_buff;
            delete[] old_file_buff;
            return false;
        }
        
        if(new_file_read == 0)
        {
            break;
        }
        reverse_content_in_file(new_file_buff,blk_size);

        //comparing the file contents in both the files and if they are not equal then return error..

        if(memcmp(new_file_buff,old_file_buff,new_file_read) != 0)
        {
            fprintf(stderr,"\nThe process of reading and comparing the file contents done till %lld and then mismatch occurs in the file contents after here.\n",total_data_read);
            delete[] new_file_buff;
            delete[] old_file_buff;
            return false;
        }
        total_data_read+=new_file_read;
   
    }
    delete[] new_file_buff;
    delete[] old_file_buff;
    return true;
}

//verifying the full file reversal flag ....

bool fullfile_verification(int new_fd,int old_fd,long long f_size)
{
    //if the file size is 0 return true because empty files are always equal...

    if(f_size == 0)
    {
        return true;
    }
    char new_file_buff[BUFFER_SIZE],old_file_buff[BUFFER_SIZE];

    
    //checking the contents chunk wise from the given files......

    long long remaining_bytes =f_size;

    if(lseek(old_fd,0,SEEK_SET) == -1){
       perror("\n Error occurred while accesing the file pointer.\n");
        return false;
    }
    while(remaining_bytes > 0)
    {
        long long chunk=remaining_bytes > BUFFER_SIZE ? BUFFER_SIZE : remaining_bytes;
         
         if(lseek(new_fd,remaining_bytes-chunk,SEEK_SET) ==-1)
         {
            perror("\nError accessing the file pointer\n");
            
            return false;
         }
         ssize_t data_read_from_new=read(new_fd,new_file_buff,chunk);
         /*  if(data_read_from_new == -1)
           {
              perror("\nError accessing the file contents.\n");
              
              return false;
           }*/
         reverse_content_in_file(new_file_buff,chunk);
         
         ssize_t data_read_from_old = read(old_fd,old_file_buff,data_read_from_new);
         /*if(data_read_from_old == -1)
         {
            perror("\nError accessing the file contents.\n");
            
            return false;
         }*/
         
         if(data_read_from_new != data_read_from_old)
         {
            
            return false;
         }
         if(data_read_from_new == 0)
            break;

        if(memcmp(new_file_buff,old_file_buff,data_read_from_new) != 0)
        {
            fprintf(stderr,"\nthe contents in both files are not equal.\n");
            return false;
        }
        remaining_bytes-=data_read_from_new;
    }



    return true;
    
}

//checking the partial file verification using the given inputs.... 

bool partial_file_verification(int new_fd, int old_Fd, long long frst, long long scnd,long long f_size)
{
    char new_file_buff[BUFFER_SIZE],old_file_buff[BUFFER_SIZE];
    if(lseek(old_Fd,0,SEEK_SET) == -1)
    {
        perror("\nError accessing the file pointer\n");
        return false;
    }
    long long first_end=frst;
    while(first_end > 0)
    {
        long long chunk= first_end > BUFFER_SIZE ? BUFFER_SIZE : first_end;
        ssize_t data_read_from_old = read(old_Fd,old_file_buff,chunk);
        if(data_read_from_old <=0)
        {
            perror("\nerror while reading the data from the file: part1(old_file)\n");
            return false;
        }
        if(lseek(new_fd,first_end-chunk,SEEK_SET) ==-1)
        {
            perror("\nError accessing the file pointer\n");
            return false;
        }
        ssize_t data_read_from_new=read(new_fd,new_file_buff,chunk);
        if(data_read_from_new <= 0)
        {
            perror("\nerror while reading the data from the file: part1(new_file)\n");
            return false;
        }

        if(data_read_from_new != data_read_from_old)
        {
            fprintf(stderr,"data in both files are of not the same size.\n");
            return false;
        }
        reverse_content_in_file(old_file_buff,data_read_from_old);

        if(memcmp(new_file_buff,old_file_buff,data_read_from_new) == 0)
        {
            fprintf(stderr,"data in both the files doesn't match.-->part-1\n");
            return false;
        }
        first_end-=data_read_from_new;
    }

    if(lseek(new_fd,frst,SEEK_SET) ==-1)
    {
        perror("\nerror seeking the position in newfile --->part-2\n");
        return false;
    }
    if(lseek(old_Fd,frst,SEEK_SET) ==-1)
    {
        perror("\nerror seeking the position in old file --->part-2\n");
        return false;
    }
    long long data_bytes_to_be_read=scnd-frst;
    while(data_bytes_to_be_read>0)
    {
        long long chunk=std::min((long long)BUFFER_SIZE,data_bytes_to_be_read);
        ssize_t data_read_from_new=read(new_fd,new_file_buff,chunk);
        if(data_read_from_new <=0)
        {
            perror("\nerror occured while reading the new_file--->part(2)\n");
            return false;
        }
        ssize_t data_read_from_old=read(old_Fd,old_file_buff,chunk);
        if(data_read_from_old <=0)
        {
            perror("\nerror occured while reading the old_file--->part(2)\n");
            return false;
        }
        if(data_read_from_new != data_read_from_old)
        {
            fprintf(stderr,"\ncontent in both thee files are not same(sizes).--->part(2)\n");
            return false;
        }
        if(memcmp(new_file_buff,old_file_buff,data_read_from_new) !=0)
        {
            fprintf(stderr,"\ncomparison failed because they are not same..--->part(2)\n");
            return false;
        }
        data_bytes_to_be_read-=data_read_from_new;

    }

    if(lseek(old_Fd,scnd,SEEK_SET) == -1)
    {
        perror("\nerror occurred while seeking the pointer index.--->part-3\n");
        return false;
    }
    long long end_ind=f_size;
    while(end_ind > scnd)
    {
        long long chunk=BUFFER_SIZE > end_ind-scnd ? BUFFER_SIZE : end_ind-scnd;
        ssize_t data_read_from_old=read(new_fd,new_file_buff,chunk);
        if(data_read_from_old <=0)
        {
            perror("\nerror occurred while reading the old file.--->part-3\n");
            return false;
        }
        if(lseek(new_fd,end_ind-data_read_from_old,SEEK_SET) ==-1)
        {
            perror("\nError sccessing the seek pointer in new file.--->part-3\n");
        }
        ssize_t data_read_from_new=read(old_Fd,old_file_buff,end_ind-data_read_from_old);
        if(data_read_from_new <=0)
        {
            perror("\nerror occurred while reading the new file.--->part-3\n");
            return false;
        }
        
        if(data_read_from_new != data_read_from_old)
        {
            fprintf(stderr,"\ndata contents in the file doesn't match\n");
            return false;

        }
        reverse_content_in_file(old_file_buff,data_read_from_old);
        if(memcmp(new_file_buff,old_file_buff,data_read_from_new) != 0)
        {
            fprintf(stderr,"\nThe file contents in boht file are not equal.--->part-3\n");
            return false;
        }
        end_ind-=data_read_from_new;
    }
    
    return true;
}

void checking_permissions_of_file(const char *file_path)
{
    struct stat file_Stats;
    if(stat(file_path,&file_Stats) == -1)
    {
        fprintf(stderr,"\nfrom the file path provided could not be able to get the stats, provided file path is %s\n",file_path);
        return;
    }
    //permissions for users...
    printf("User has read permission on %s: %s\n",file_path,(file_Stats.st_mode & S_IRUSR) ? "YEs" : "No");
    printf("User has write permission on %s: %s\n",file_path,(file_Stats.st_mode & S_IWUSR) ? "YEs" : "No");
    printf("User has execute permission on %s: %s\n",file_path,(file_Stats.st_mode & S_IXUSR) ? "YEs" : "No");

    //permissions for group....
    printf("Group has read permission on %s: %s\n",file_path,(file_Stats.st_mode & S_IRGRP) ? "YEs" : "No");
    printf("Group has write permission on %s: %s\n",file_path,(file_Stats.st_mode & S_IWGRP) ? "YEs" : "No");
    printf("Group has execute permission on %s: %s\n",file_path,(file_Stats.st_mode & S_IXGRP) ? "YEs" : "No");

    //permissions for others....
    printf("Others has read permission on %s: %s\n",file_path,(file_Stats.st_mode & S_IROTH) ? "YEs" : "No");
    printf("Others has write permission on %s: %s\n",file_path,(file_Stats.st_mode & S_IWOTH) ? "YEs" : "No");
    printf("Others has execute permission on %s: %s\n",file_path,(file_Stats.st_mode & S_IXOTH) ? "YEs" : "No");



}

int main(int argc,char *argv[])
{
    //The no of arguments the user entered must be greater than or equal to 4...
    if(argc < 4)
    {
        fprintf(stderr,"Enter sufficient number of arguments...");
        return 1;
    }
    int flag=atoi(argv[4]);
    //checking no of arguments passed for respective flags...
    if ((flag == 0 && argc != 6) || (flag == 1 && argc != 5) || (flag == 2 && argc != 7))
    {
        fprintf(stderr,"The flag specified and the arguments passed for the respective flag doesn't matches");
        return 1;
    }
    //getting the files and directory paths from the user input and assigning to the variables.
    const char* q1_generated_file=argv[1];
    const char* q1_input_file=argv[2];
    const char* direcotry=argv[3];

    //verifying whether the directory and the files that should exist in the directory are present or not...

    struct stat new_file_stat,old_file_stat,dir_stat;
    bool directory_created = (stat(direcotry, &dir_stat) ==0);
    bool files_exist=(stat(q1_generated_file, &new_file_stat) ==0 && stat(q1_input_file, &old_file_stat) == 0);
    bool size_equal=files_exist && (new_file_stat.st_size ==old_file_stat.st_size);

    bool content_verification=false;
    if(files_exist && size_equal)
    {
        int new_file_fd=open(q1_generated_file,O_RDONLY);
        int old_file_fd=open(q1_input_file,O_RDONLY);
        if(new_file_fd != -1 && old_file_fd != -1)
        {
            switch (flag)
            {
                case 0:
                {
                    content_verification = blockwise_verification(new_file_fd, old_file_fd, atol(argv[5]));
                    break;
                }
                case 1:
                {
                    content_verification = fullfile_verification(new_file_fd, old_file_fd, new_file_stat.st_size);
                    break;
                }
                case 2:
                {
                    content_verification = partial_file_verification(new_file_fd, old_file_fd, atoll(argv[5]), atoll(argv[6]), new_file_stat.st_size);
                    break;
                }
                 default:
                    fprintf(stderr,"The entered flag value is not a valid flag value so please enter the appropriate flag value like 0,1,2.\n");
                    break;
            }
            
        }
        if(new_file_fd != -1) close(new_file_fd);
        if(old_file_fd != -1) close(old_file_fd);
    }

    //the required output creation after all verifications done...

    printf("Directory id created: %s\n",directory_created ? "Yes" : "No" );
    if(content_verification)
    {
       printf("Whether file contents are correctly processed : %s\n", "Yes");
    }
    else
    {
       printf("Whether file contents are correctly processed : %s\n", "No");
    }
    printf("Both Files Sizes are Same : %s\n",size_equal ? "yes" : "No");
    
    //checking all the permissons of the file like read, write, execure for users, group, others...

    checking_permissions_of_file(q1_generated_file);

    checking_permissions_of_file(q1_input_file);

    checking_permissions_of_file(direcotry);


}
