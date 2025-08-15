#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<errno.h>
#include<fcntl.h>
#include <new>
#define BUFFER_SIZE 8192

//function to show the progress like how much percentage of the file is processed... 

void progress_updater(long long curr_bytes, long long total_bytes, int& progress)
{
    if (curr_bytes==0)
    {
        return;
    }
    int percentage=(curr_bytes*100)/total_bytes;
    if (percentage > progress || percentage==100)
    {
        printf("\rProgress : %d%%",percentage);
        fflush(stdout);
        progress=percentage;
    }

}

//implementing the reverse function to reverse the file contents like the contents in the buffer or chunk wise reversing of the file...
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
//implementing the blockwise reversal function ---> flag 0

int blockwise_reversal(int inpt_fd,int out_fd, long blk_size, long long f_size)
{
    //validating the block size, block sizze should not be 0...
    if (blk_size <= 0)
    {
        fprintf(stderr,"\nBlock size cannot be less than or equal to zero\n");
        return 0;
    }
    // handling memory limit errors using exception handling...
    char* buff=nullptr;
    try{
        buff=new char[blk_size];
    }
    catch (const std::bad_alloc& e){
        fprintf(stderr, " \n Error Occurred : Memory allocation for block size you have mentioned is %ld, which is larger than the avilable RAM, so try giving appropriate block size with in limits.\n",blk_size);
        return 0;
    }
    long long bytes_prosessed_till_now=0;
    int progress=-1;

    progress_updater(0,f_size,progress);

    while(1)
    {
        ssize_t bytes_read_till_now = read(inpt_fd,buff,blk_size);
        if(bytes_read_till_now == -1)
        {
            perror("\nError occurred while reading the file");
            delete[] buff;
            return 0;

        }
        if (bytes_read_till_now == 0)
        {
            break;
        }
        reverse_content_in_file(buff,bytes_read_till_now);

        ssize_t bytes_written=write(out_fd,buff,bytes_read_till_now);
        //checking whether data read from the input file and the data written to the output file are equal or not .... 
        if(bytes_written != bytes_read_till_now)
        {
            perror("\nError occurred while writing to the output file\n");
            delete[] buff;
            return 0;
        }
        bytes_prosessed_till_now+=bytes_written;
        progress_updater(bytes_prosessed_till_now,f_size,progress);
    }
    return 1;
}

//implementing the function to do full file reversal(flag 1)...

int full_file_reversal(int inpt_fd,int output_fd,long long f_size)
{
    //checking if the provided file having any information if it consists of some information then we will proceed to modify the data...
    if(f_size==0)
    {
        return 0;
    }
    long long cur_pos=f_size;
    char buff[BUFFER_SIZE];
    long long data_processed_till_now=0;
    int progress=-1;
    

    progress_updater(data_processed_till_now,f_size,progress);
    //here we implement the ligic to reverse all the file contents...
    while(cur_pos > 0)
    {
    //dividing the based on the chunks so that it allows smooth execution of large files which may exceed the RAM sizze  ...

        long long chunk_size= cur_pos > BUFFER_SIZE ? BUFFER_SIZE : cur_pos;
        off_t start_pos_chunk=lseek(inpt_fd,cur_pos-chunk_size,SEEK_SET);
        if(start_pos_chunk == -1)
        {
            perror("\nError occurred while getting the index of start position of the chunk.\n");
            return 0;
        }

        ssize_t read_data_after_seek = read(inpt_fd,buff,chunk_size);

        if (read_data_after_seek == -1)
        {
            perror("\nError accessing the contents in the input file.\n");
            return 0;

        }
        // reversing the contents in the buffer to get the desired reversed output as expected...
        reverse_content_in_file(buff,read_data_after_seek);
        ssize_t write_data_from_in_fd=write(output_fd,buff,read_data_after_seek);
        if(write_data_from_in_fd != read_data_after_seek)
        {
            perror("\nError occurred while writing to the output file.\n");
            return 0;
        }

        data_processed_till_now = data_processed_till_now + write_data_from_in_fd;
        cur_pos=cur_pos-read_data_after_seek;

        progress_updater(data_processed_till_now,f_size,progress);


    }
    return 1;


}
//implementing the function for partial file reversal...

int partial_file_reversal(int inpt_fd,int out_fd,long long first,long long second,long long f_size)
{
    //validating the indices given by the user...
    if (first < 0 || second < 0 || first >= f_size || second >= f_size || first > second)
    {
        fprintf(stderr,"\nThe entered indices are inappropriate like whether they are out of bound from the file size or may be first index greater than the second index\n");
        return 0;
    }
    
    //here in this function we divide the whole code into three parts and then solve each part to get the required data in the file...  
    long long data_processed=0;
    int progress=-1;
    char buff[BUFFER_SIZE];

    progress_updater(data_processed,f_size,progress);
    
    //reversing the file contents till first index provided by the user...
    long long first_ind=first;
    while(first_ind > 0)
    {
        long long chunk=first_ind > BUFFER_SIZE ? BUFFER_SIZE : first_ind;
        if(lseek(inpt_fd,first_ind-chunk,SEEK_SET) == -1)
        {
            perror("\n Error while accessing or moving the pointer to desired location(1)\n");
            return 0;
        }
        ssize_t data_read=read(inpt_fd,buff,chunk);
        if(data_read <= 0)
        {
            perror("\n Error occurred while accessing the input file(1)\n");
            return 0;
        }
        reverse_content_in_file(buff,data_read);
        if(write(out_fd,buff,data_read) != data_read)
        {
            perror("\nError occurred while wwriting the file(1)\n");
            return 0;
        }

        data_processed+=data_read;
        first_ind-=data_read;
        progress_updater(data_processed,f_size,progress);

    } 
    
    //from here the function will operate on the data from first index to second index (actually no modification needed in this portion so we just write the file contents in input file directly to th eoutput file)... 
    if(lseek(inpt_fd,first,SEEK_SET) == -1)
    {
        perror("\nError occurred while trying to access the middle part(middle)\n");
        return 0;
    }

    long long middle_size=second-first;
    
    while(middle_size > 0)
    {
        long long chunk=middle_size > BUFFER_SIZE ? BUFFER_SIZE : middle_size;
        if(lseek(inpt_fd,first,SEEK_SET) == -1)
        {
            perror("\nError getting index for accessing data(middle)\n");
            return 0;
        }
        ssize_t data_read=read(inpt_fd,buff,chunk);
        if(data_read <= 0)
        {
            perror("\nError while reading the file(middle)\n");
            return 0;
        }
        if(write(out_fd,buff,data_read) != data_read)
        {
            perror("\nError while writing the file(middle)\n");
            return 0;
        }

        data_processed+=data_read;
        middle_size-=data_read;
        progress_updater(data_processed,f_size,progress);
    }
    
    //here we implement logic to modify the contents from ssecond index till last index of the file...
    long long end_ind=f_size;
    while(end_ind > second)
    {
        long long chunk=end_ind-second > BUFFER_SIZE ? BUFFER_SIZE : end_ind-second;
        if(lseek(inpt_fd,end_ind-chunk,SEEK_SET) == -1)
        {
            perror("\nError while getting index using lseek()\n");
            return 0;
        }
        ssize_t data_read=read(inpt_fd,buff,chunk);
        if(data_read <= 0)
        {
            perror("\nError while reading the data from input file\n");
            return 0;
        }
        reverse_content_in_file(buff,data_read);
        if(write(out_fd,buff,data_read) != data_read)
        {
            perror("\nError occurred while writing the file from buffer(3)\n");
            return 0;
        }

        data_processed+=data_read;
        end_ind-=data_read;
        progress_updater(data_processed,f_size,progress);
    }
    
    return 1;
}

int main(int argc, char *argv[]){

    //verify the number of arguments like whether the user entered the required arguments or not...

    if (argc < 3)
    {
        fprintf(stderr,"Usage: %s <input_file> <flag>\n",argv[0]);
        return 1;
    }
    const char *input_file=argv[1];
    int flag = atoi(argv[2]);
    
    //checking the arguments for each case whether the user provided correct number of arguments for each flag or not...

    if(flag == 0 && argc != 4){
        fprintf(stderr,"Incorrect number of arguments for flag 0");
        return 1;
    }
    if(flag == 1 && argc != 3){
        fprintf(stderr,"incorrect number of arguments given for flag 1");
        return 1;
    }
    if(flag==2 && argc != 5){
        fprintf(stderr," incorrect number of arguments provided for flag 2");
        return 1;
    }

    //creating the directory
    const char *Directory_Name = "Assignment1";

    if (mkdir(Directory_Name,0700)==-1 && errno != EEXIST)
    {
        perror("Error occurred while creating the directory");
        return 1;
    } 

    //constructing the file path to store the output files...
    std::string output_file_path = std::string(Directory_Name) + "/" +  std::to_string(flag) + "_"+ input_file;

    //now we will open the input file ans then give read permissions to the user...
    int input_file_discriptor = open(input_file, O_RDONLY);
    if (input_file_discriptor == -1)
    {
        perror("Error occured while opening the input file");
        return 1;
    }

    //now we will open the output file and give permissions for it...
    int output_file_discriptor = open(output_file_path.c_str(),O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (output_file_discriptor == -1)
    {
        perror("Error occurred while opening the output file");
        close(input_file_discriptor);
        return 1;
    }

    //now we need to read the input file size so that we can use this info for progress if the file that is done..
    struct stat input_file_info;
    if (fstat(input_file_discriptor, &input_file_info) == -1)
    {
        perror("Error occurred while getting the meta data of the file that contains the information like size,permissions,modifications");
        close(input_file_discriptor);
        close(output_file_discriptor);
        return 1;
    }
    long long total_file_size=input_file_info.st_size;
    int success=0;

    //now we will execute the flag operations based on the user input like we implement and call functions for all the three flags using switch and for default we just send error message to the user...
    switch(flag)
    {
        case 0:
        {
            long blk_size = atol(argv[3]);
            if(blockwise_reversal(input_file_discriptor,output_file_discriptor,blk_size,total_file_size))
                success=1;
            break;
        }
        case 1:
        {
            if(full_file_reversal(input_file_discriptor,output_file_discriptor,total_file_size))
                success=1;
            break;
        }
        case 2:
        {
            long long int first = atol(argv[3]);
            long long int second = atol(argv[4]);
            if(partial_file_reversal(input_file_discriptor,output_file_discriptor,first,second,total_file_size))
                success = 1;
            break;
        }
        default:
        fprintf(stderr,"The entered flag value is not a valid flag value so please enter the appropriate flag value like 0,1,2.\n");
        break;
    }
// closing the files that were open before so that there will be no resource wastage...
    close(input_file_discriptor);
    close(output_file_discriptor);

    if(success)
        std::cout<<"Successfully File reversal completed and you can find the output file at "<< output_file_path <<std::endl;
    else
        std::cout<<"\nProgram terminated due to Error!!!\n";

    return 0;

}
