#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <unistd.h>


// divide the file into chunks

typedef struct
{
    char *start;
    long size;
    int *out_counts;
    char *out_chars;
    long num_pairs;

} chunk;

void compress(char *start, long size, int *out_counts, char *out_chars, long *out_size)
{
    long pos = 0;
    long i = 0;
    while (i < size)
    {
        char current = start[i];
        int count = 1;

        // count how many times this char repeats
        while (i + count < size && start[i + count] == current)
        {
            count++;
        }

        out_counts[pos] = count;
        out_chars[pos] = current;
        pos++;

        i += count;
    }

    *out_size = pos; // how many pairs we found
}

// threads requres function shapes so the thread_compress func is the sol for that
void *thread_compress(void *pt)
{
    chunk *c = (chunk *)pt;
    c->out_counts = malloc(c->size * sizeof(int));
    c->out_chars = malloc(c->size * sizeof(char));
    compress(c->start, c->size, c->out_counts, c->out_chars, &c->num_pairs);

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "pzip: file1 [file2 ...]\n");
        exit(1);
    }

    int num_threads = get_nprocs();

    // calculate total size of all files
    long total_size = 0;
    int *fds = malloc((argc - 1) * sizeof(int));
    long *sizes = malloc((argc - 1) * sizeof(long));

    for (int i = 1; i < argc; i++)
    {
        fds[i - 1] = open(argv[i], O_RDONLY);
        if (fds[i - 1] == -1)
        {
            fprintf(stderr, "pzip: cannot open file\n");
            exit(1);
        }
        struct stat st;
        fstat(fds[i - 1], &st);
        sizes[i - 1] = st.st_size;
        total_size += st.st_size;
    }

    if (num_threads > total_size)
        num_threads = total_size;

    // mmap all files and copy into one big buffer
    char *data = malloc(total_size);
    long offset = 0;
    for (int i = 0; i < argc - 1; i++)
    {
        char *mapped = mmap(NULL, sizes[i], PROT_READ, MAP_PRIVATE, fds[i], 0);
        if (mapped == MAP_FAILED)
        {
            fprintf(stderr, "pzip: mmap failed\n");
            exit(1);
        }
        memcpy(data + offset, mapped, sizes[i]);
        munmap(mapped, sizes[i]);
        close(fds[i]);
        offset += sizes[i];
    }

    long size = total_size;
    
    long chunk_size = size / num_threads;
    chunk *chunks = malloc(num_threads * sizeof(chunk));

    for (int i = 0; i < num_threads; i++)
    {
        chunks[i].start = data + (i * chunk_size);
        chunks[i].size = chunk_size;
    }

    // the leftover bytes added to lust cuhck
    chunks[num_threads - 1].size += size % num_threads;

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, thread_compress, &chunks[i]);
    }

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // fix the boundaries problem between chuncks checks if (last char of chuck = first of the next one)
    // collect everything into one flat list
    int *final_counts = malloc(size * sizeof(int));
    char *final_chars = malloc(size * sizeof(char));
    long final_size = 0;

    for (int i = 0; i < num_threads; i++)
    {
        for (long j = 0; j < chunks[i].num_pairs; j++)
        {
            char c = chunks[i].out_chars[j];
            int n = chunks[i].out_counts[j];

            // if same char as last one, just add the count
            if (final_size > 0 && final_chars[final_size - 1] == c)
            {
                final_counts[final_size - 1] += n;
            }
            else
            {
                final_counts[final_size] = n;
                final_chars[final_size] = c;
                final_size++;
            }
        }
    }

    // write output
    for (long i = 0; i < final_size; i++)
    {
        fwrite(&final_counts[i], sizeof(int), 1, stdout);
        fwrite(&final_chars[i], sizeof(char), 1, stdout);
    }


    for (int i = 0; i < num_threads; i++) {
        free(chunks[i].out_counts);
        free(chunks[i].out_chars);
    }
    free(chunks);
    free(threads);
    free(data);
    free(fds);
    free(sizes);


    return 0;
}