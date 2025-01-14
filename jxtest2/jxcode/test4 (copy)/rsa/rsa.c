#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ACCURACY 5
#define SINGLE_MAX 10000
#define EXPONENT_MAX 1000
#define BUF_SIZE 1024
unsigned long urdtsc(void)
{
    unsigned int lo,hi;

    __asm__ __volatile__
    (
        "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long)hi<<32|lo;
}
/**
 * Computes a^b mod c\uff0c\u8ba1\u7b97a^b mod c
 */
int modpow(long long a, long long b, int c) {
    int res = 1;
    while(b > 0) {
        /* Need long multiplication else this will overflow...
         \u5fc5\u987b\u4f7f\u7528\u957f\u4e58\u6cd5\uff0c\u5426\u5219\u8fd9\u5c06\u6ea2\u51fa*/
        if(b & 1) {
            res = (res * a) % c;
        }
        b = b >> 1;
        a = (a * a) % c; /* Same deal here */
    }
    return res;
}

/**
 * Computes the Jacobi symbol, (a, n)\uff0c\u8ba1\u7b97Jacobi\u7b26\u53f7\uff08a,n\uff09
 */
int jacobi(int a, int n) {
    int twos, temp;
    int mult = 1;
    while(a > 1 && a != n) {
        a = a % n;
        if(a <= 1 || a == n) break;
        twos = 0;
        while(a % 2 == 0 && ++twos) a /= 2; /* Factor out multiples of 2 ,\u51cf\u53bb2\u7684\u500d\u6570*/
        if(twos > 0 && twos % 2 == 1) mult *= (n % 8 == 1 || n % 8 == 7) * 2 - 1;
        if(a <= 1 || a == n) break;
        if(n % 4 != 1 && a % 4 != 1) mult *= -1; /* Coefficient for flipping\uff0c\u7ffb\u8f6c\u7cfb\u6570 */
        temp = a;
        a = n;
        n = temp;
    }
    if(a == 0) return 0;
    else if(a == 1) return mult;
    else return 0; /* a == n => gcd(a, n) != 1 */
}

/**
 * Check whether a is a Euler witness for n\uff0c\u68c0\u67e5a\u662f\u5426\u4e3an\u7684\u6b27\u62c9\u89c1\u8bc1 */
int solovayPrime(int a, int n) {
    int x = jacobi(a, n);
    if(x == -1) x = n - 1;
    return x != 0 && modpow(a, (n - 1)/2, n) == x;
}

/**
 * Test if n is probably prime, using accuracy of k (k solovay tests)\uff0c\u7528k\u7684\u7cbe\u5ea6\u68c0\u67e5n\u662f\u5426\u53ef\u80fd\u662f\u7d20\u6570 */
int probablePrime(int n, int k) {
    if(n == 2) return 1;
    else if(n % 2 == 0 || n == 1) return 0;
    while(k-- > 0) {
        if(!solovayPrime(rand() % (n - 2) + 2, n)) return 0;
    }
    return 1;
}

/**
 * Find a random (probable) prime between 3 and n - 1\u57283\u548c\uff08n-1\uff09\u4e4b\u95f4\u627e\u4e00\u4e2a\u968f\u673a\u7d20\u6570, this distribution is\uff0c
 * nowhere near uniform, see prime gaps
 */
int randPrime(int n) {
    int prime = rand() % n;
    n += n % 2; /* n needs to be even so modulo wrapping preserves oddness */
    prime += 1 - prime % 2;
    while(1) {
        if(probablePrime(prime, ACCURACY)) return prime;
        prime = (prime + 2) % n;
    }
}

/**
 * Compute gcd(a, b)\uff0c\u8ba1\u7b97gcd\uff08a,b\uff09
 */
int gcd(int a, int b) {
    int temp;
    while(b != 0) {
        temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Find a random exponent x between 3 and n - 1 such that gcd(x, phi) = 1,\u57283\u548cn-1\u4e4b\u95f4\u627e\u5230\u968f\u673a\u6307\u6570x\uff0c\u4f7f\u5f97gcd(x,phi)=1
 * this distribution is similarly nowhere near uniform,\u8fd9\u79cd\u5206\u5e03\u540c\u6837\u4e0d\u63a5\u8fd1\u5236\u670d
 */
int randExponent(int phi, int n) {
    int e = rand() % n;
    while(1) {
        if(gcd(e, phi) == 1) return e;
        e = (e + 1) % n;
        if(e <= 2) e = 3;
    }
}

/**
 * Compute n^-1 mod m by extended euclidian method\uff0c\u7528\u6269\u5c55\u6b27\u51e0\u91cc\u5f97\u6cd5\u8ba1\u7b97n^-1 mod m
 */
int inverse(int n, int modulus) {
    int a = n, b = modulus;
    int x = 0, y = 1, x0 = 1, y0 = 0, q, temp;
    while(b != 0) {
        q = a / b;
        temp = a % b;
        a = b;
        b = temp;
        temp = x; x = x0 - q * x; x0 = temp;
        temp = y; y = y0 - q * y; y0 = temp;
    }
    if(x0 < 0) x0 += modulus;
    return x0;
}

/**
 * Read the file fd into an array of bytes ready for encryption.\u5c06\u6587\u4ef6fd\u8bfb\u5165\u51c6\u5907\u52a0\u5bc6\u7684\u5b57\u8282\u6570\u7ec4
 * The array will be padded with zeros until it divides the number of\u6570\u7ec4\u5c06\u586b\u5145\u96f6\uff0c\u77e5\u9053\u5b83\u5212\u5206\u6bcf\u4e2a\u5757\u52a0\u5bc6\u7684\u5b57\u8282\u6570
 * bytes encrypted per block. Returns the number of bytes read.\u8fd4\u56de\u8bfb\u53d6\u7684\u5b57\u8282\u6570
 */
int readFile(FILE* fd, char** buffer, int bytes) {
    int len = 0, cap = BUF_SIZE, r;
    char buf[BUF_SIZE];
    *buffer = (char *)malloc(BUF_SIZE * sizeof(char));
    while((r = fread(buf, sizeof(char), BUF_SIZE, fd)) > 0) {
        if(len + r >= cap) {
            cap *= 2;
            *buffer = (char *)realloc(*buffer, cap);
        }
        memcpy(&(*buffer)[len], buf, r);
        len += r;
    }
    /* Pad the last block with zeros to signal end of cryptogram. An additional block is added if there is no room\uff0c\u5c06\u6700\u540e\u4e00\u4e2a\u5e26\u6709\u96f6\u7684\u5757\u63d2\u5165\u5bc6\u7801\u7684\u4fe1\u53f7\u7aef\u3002 \u5982\u679c\u6ca1\u6709\u623f\u95f4\uff0c\u5219\u589e\u52a0\u4e00\u4e2a\u989d\u5916\u7684\u8857\u533a */
    if(len + bytes - len % bytes > cap) *buffer = (char *)realloc(*buffer, len + bytes - len % bytes);
    do {
        (*buffer)[len] = '\0';
        len++;
    }
    while(len % bytes != 0);
    return len;
}

/**
 * Encode the message m using public exponent and modulus, c = m^e mod n\u4f7f\u7528\u516c\u5171\u6307\u6570\u548c\u6a21\u91cf\u5bf9\u6d88\u606fm\u8fdb\u884c\u7f16\u7801\uff0cc = m^e Mod n
 */
int encode(int m, int e, int n) {
    return modpow(m, e, n);
}

/**
 * Decode cryptogram c using private exponent and public modulus, m = c^d mod n\uff0c\u7528\u79c1\u6709\u6307\u6570\u548c\u516c\u5171\u6a21\u91cf\u89e3\u7801\u5bc6\u7801c\uff0cm = c^d Mod n
 */
int decode(int c, int d, int n) {
    return modpow(c, d, n);
}

/**
 * Encode the message of given length, using the public key (exponent, modulus)
 * The resulting array will be of size len/bytes, each index being the encryption
 * of "bytes" consecutive characters, given by m = (m1 + m2*128 + m3*128^2 + ..),
 * encoded = m^exponent mod modulus
 * \u4f7f\u7528\u516c\u94a5\uff08\u6307\u6570\u3001\u6a21\u6570)\u5bf9\u7ed9\u5b9a\u957f\u5ea6\u7684\u6d88\u606f\u8fdb\u884c\u7f16\u7801\uff09
 \u5f97\u5230\u7684\u6570\u7ec4\u5c06\u662f\u5927\u5c0f\u4e3alen/\u5b57\u8282\uff0c\u6bcf\u4e2a\u7d22\u5f15\u662f\u7531m=(m1m2*128m3*128^2.)\u7ed9\u51fa\u7684\u201c\u5b57\u8282\u201d\u8fde\u7eed\u5b57\u7b26\u7684\u52a0\u5bc6\uff0c\u7f16\u7801=m^\u6307\u6570mod\u6a21\u91cf
 */
int* encodeMessage(int len, int bytes, char* message, int exponent, int modulus) {
    int *encoded = (int *)malloc((len/bytes) * sizeof(int));
    int x, i, j;
    for(i = 0; i < len; i += bytes) {
        x = 0;
        for(j = 0; j < bytes; j++) x += message[i + j] * (1 << (7 * j));
        encoded[i/bytes] = encode(x, exponent, modulus);
#ifndef MEASURE
        printf("%d ", encoded[i/bytes]);
#endif
    }
    return encoded;
}

/**
 * Decode the cryptogram of given length, using the private key (exponent, modulus)
 * Each encrypted packet should represent "bytes" characters as per encodeMessage.
 * The returned message will be of size len * bytes.
 * \u4f7f\u7528\u79c1\u94a5\uff08\u6307\u6570\u3001\u6a21\u6570)\u89e3\u7801\u7ed9\u5b9a\u957f\u5ea6\u7684\u5bc6\u7801\uff09
 \u6bcf\u4e2a\u52a0\u5bc6\u7684\u6570\u636e\u5305\u5e94\u8be5\u6309\u7167\u7f16\u7801\u6d88\u606f\u8868\u793a\u201c\u5b57\u8282\u201d\u5b57\u7b26\u3002
 \u8fd4\u56de\u7684\u6d88\u606f\u5927\u5c0f\u4e3alen*\u5b57\u8282\u3002
 */
int* decodeMessage(int len, int bytes, int* cryptogram, int exponent, int modulus) {
    int *decoded = (int *)malloc(len * bytes * sizeof(int));
    int x, i, j;
    for(i = 0; i < len; i++) {
        x = decode(cryptogram[i], exponent, modulus);
        for(j = 0; j < bytes; j++) {
            decoded[i*bytes + j] = (x >> (7 * j)) % 128;
#ifndef MEASURE
            if(decoded[i*bytes + j] != '\0') printf("%x ", decoded[i*bytes + j]);
#endif
        }
    }
    return decoded;
}
int main(){
    unsigned long t1,t2,t;
    int p, q, n, phi, e, d, bytes, len;
     int *encoded, *decoded;
     char *buffer;
     FILE *f;
     
     t1=urdtsc();
     srand(time(NULL));
     while(1) {
         p = randPrime(SINGLE_MAX);  
         q = randPrime(SINGLE_MAX);
         n = p * q;
         
         if(n < 128) {

         }
         else break;
     }
     if(n >> 21) bytes = 3;
     else if(n >> 14) bytes = 2;
     else bytes = 1;
    
  
     phi = (p - 1) * (q - 1);
     
  
     e = randExponent(phi, EXPONENT_MAX);
     
  
     d = inverse(e, phi);
     
    

     
     f = fopen("text.txt", "r");
     
     //t2=urdtsc();
     //t=t2-t1;
     
      
      
    // t1=urdtsc();
     len = readFile(f, &buffer, bytes); /* len will be a multiple of bytes, to send whole chunks\u4f26\u5c06\u662f\u591a\u4e2a\u5b57\u8282\uff0c\u4ee5\u53d1\u9001\u6574\u4e2a\u5757 */
     encoded = encodeMessage(len, bytes, buffer, e, n);
     t2=urdtsc();
     t=(t2-t1)*5/17*2;
    // printf("\nt=%ld\n",t*5/17);
     if(t<160000&&t>110000){
     FILE *file1;
     file1 = fopen("ttt.txt","a");
     fprintf(file1,"%ld\n",t);
     fclose(file1);
     //printf("\n\u7f16\u7801\u6210\u529f\u5b8c\u6210 ... ");
     //printf("\u6587\u4ef6 \"text.txt\" \u8bfb\u53d6\u6210\u529f, \u8bfb\u53d6\u5230%d\u5b57\u8282. \u4ee5%d\u5b57\u8282\u7684\u5b57\u8282\u6d41\u7f16\u7801 ... ", len, bytes);
     fclose(f);
     }
    // getchar();
  
  
     //printf("\u6b63\u5728\u89e3\u7801\u7f16\u7801\u7684\u4fe1\u606f ... ");
     //getchar();
     //printf("\u89e3\u7801\u5f97\u660e\u6587\u4e3a\uff1a");
     decoded = decodeMessage(len/bytes, bytes, encoded, d, n);
  
  
     //printf("\nRSA\u7b97\u6cd5\u6f14\u793a\u5b8c\u6210!\n");
  
     free(encoded);
     free(decoded);
     free(buffer);
     return EXIT_SUCCESS;
}
