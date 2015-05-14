/**
 * EE6650 Video Signal Processing
 * Homework Assignment #4: KLT v.s. DCT
 * 100061116 Hsiao Yu-Ling
 */
#include<iostream>
#include<cmath>
#include<fstream>
#include<Eigen>
using namespace std;

unsigned char* readBMP(const char* image, fpos_t header, int width, int height){
    FILE *fimage;
    unsigned char *lumaBMP;
    unsigned char *luma;

    lumaBMP = new unsigned char[width*height];
    luma = new unsigned char[width*height];

    fimage = fopen(image, "rb");
    fsetpos(fimage, &header);
    fread(lumaBMP, sizeof(char), width*height, fimage);
    fclose(fimage);

    /// reorder
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            luma[i*width+j] = lumaBMP[(height-i-1)*width+j];
        }
    }
    delete []lumaBMP;

    return luma;
}

void printMatrix(double* matrix, int size){
    for(int row=0; row<size; row++){
        for(int col=0; col<size; col++){
            cout << matrix[row*size+col] << ' ';
        }
        cout << endl;
    }
}

double* generateCovMatrix(unsigned char* data, int dataSize, int vectorSize){
    int dataLength = dataSize/vectorSize;
    double* avg;
    double* covMatrix;
    avg = new double[vectorSize];
    covMatrix = new double[vectorSize*vectorSize];
    for(int i=0; i<vectorSize; i++){
        avg[i]=0.0;
    }
    for(int i=0; i<vectorSize*vectorSize; i++){
        covMatrix[i]=0.0;
    }
    /// calculate mean, covariance
    for(int i=0; i<dataLength; i++){
        /// sum X
        for(int j=0; j<vectorSize; j++){
            avg[j]+=data[i*vectorSize+j];
        }
        /// sum XY
        for(int row=0; row<vectorSize; row++){
            for(int col=row; col<vectorSize; col++){
                covMatrix[row*vectorSize+col]+=data[i*vectorSize+row]*data[i*vectorSize+col];
            }
        }
    }
    /// mean E(X)
    for(int i=0; i<vectorSize; i++){
        avg[i]/=dataLength;
    }
    /// covariance = E(XY)-E(X)E(Y)
    for(int row=0; row<vectorSize; row++){
        for(int col=row; col<vectorSize; col++){
            covMatrix[row*vectorSize+col]/=dataLength;
            covMatrix[row*vectorSize+col]-=avg[row]*avg[col];
        }
    }
    for(int row=0; row<vectorSize; row++){
        for(int col=0; col<row; col++){
            covMatrix[row*vectorSize+col]=covMatrix[col*vectorSize+row];
        }
    }

    delete []avg;
    return covMatrix;
}

void klt(double* covMat, int matrixSize, double* covMatDecorr, double* transformMat){
    Eigen::MatrixXf cov(matrixSize, matrixSize);

    for(int i=0; i<matrixSize; i++){
        for(int j=0; j<matrixSize; j++){
            cov(i,j) = covMat[i*matrixSize+j];
        }
    }
    //cout << "Covriance Matrix" << endl << cov << endl;
    /// SVD
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(cov, Eigen::ComputeThinU | Eigen::ComputeThinV);
    //cout << "Its singular values are:" << endl << svd.singularValues() << std::endl;
    //cout << "Transform Matrix:" << endl << svd.matrixV().transpose() << std::endl;
    //cout << svd.singularValues()(2) << endl;
    for(int i=0; i<matrixSize; i++){
        for(int j=0; j<matrixSize; j++){
            covMatDecorr[i*matrixSize+j] = (i==j) ? svd.singularValues()(i) : 0.0;
        }
    }
    for(int i=0; i<matrixSize; i++){
        for(int j=0; j<matrixSize; j++){
            transformMat[i*matrixSize+j] = svd.matrixV().transpose()(i,j);
        }
    }
}

double dct(double* covMat, int matrixSize, double* covMatDecorr){
    Eigen::MatrixXf cov(matrixSize, matrixSize);
    Eigen::MatrixXf Tdct(matrixSize, matrixSize);
    Eigen::MatrixXf D(matrixSize, matrixSize);
    double efficiency = 0.0;
    double X = 0.0;
    double Y = 0.0;

    for(int i=0; i<matrixSize; i++){
        for(int j=0; j<matrixSize; j++){
            cov(i,j) = covMat[i*matrixSize+j];
        }
    }

    /// generate DCT matrix
    for(int i=0; i<matrixSize; i++){
        for(int j=0; j<matrixSize; j++){
            Tdct(i,j) = sqrt(2.0/matrixSize)*cos(((2*j+1)*i*M_PI)/(2*matrixSize));
        }
    }
    for(int j=0; j<matrixSize; j++){
        Tdct(0,j) = Tdct(0,j)/sqrt(2);
    }
    //cout << "DCT matrix" << endl << Tdct << endl;

    /// calculate de-correlation covariance matrix
    D = Tdct*cov*Tdct.transpose();
    //cout << "D matrix" << endl << D << endl;

    for(int i=0; i<matrixSize; i++){
        for(int j=0; j<matrixSize; j++){
            covMatDecorr[i*matrixSize+j] = D(i,j);
        }
    }
    /// calculate efficiency
    for(int i=0; i<matrixSize; i++){
        for(int j=0; j<matrixSize; j++){
            if(i!=j){
                X += abs(cov(i,j));
                Y += abs(D(i,j));
            }
        }
    }
    efficiency = 1-Y/X;
    //cout << "Efficiency: " << efficiency << endl;
    return efficiency;
}

void kltVSdct(unsigned char* data, int dataSize, int vectorSize){
    double* covMat;
    double* covDecorr;
    double* transMat;
    double eff;
    covDecorr = new double[vectorSize*vectorSize];
    transMat = new double[vectorSize*vectorSize];

    /// calculate covariance matrix
    covMat = generateCovMatrix(data, dataSize, vectorSize);
    cout << "Covariance Matrix" << endl;
    printMatrix(covMat, vectorSize);
    /// use covariance matrix to calculate KLT
    klt(covMat, vectorSize, covDecorr, transMat);
    cout << "KLT:" << endl;
    cout << "De-correlation Covariance Matrix" << endl;
    printMatrix(covDecorr, vectorSize);
    cout << "Transform Matrix" << endl;
    printMatrix(transMat, vectorSize);
    /// use DCT to approximate KLT
    cout << "DCT:" << endl;
    eff = dct(covMat, vectorSize, covDecorr);
    cout << "De-correlation Covariance Matrix" << endl;
    printMatrix(covDecorr, vectorSize);
    cout << "Efficiency: " << eff << endl << endl;
    delete []data;
    delete []covMat;
    delete []covDecorr;
    delete []transMat;
}

int main()
{
    const char lenaBmp[] = "lena.bmp";
    const char baboonBmp[] = "baboon.bmp";
    fpos_t size_header = 1078;
    int width = 512;
    int height = 512;
    unsigned char* luma;
    int point = 4;

    ofstream out("out.txt");
    streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

    /// read image
    luma = readBMP(lenaBmp, size_header, width, height);
    cout << "Lena, 4 point:" << endl;
    kltVSdct(luma, width*height, point);

    luma = readBMP(baboonBmp, size_header, width, height);
    cout << "Baboon, 4 point:" << endl;
    kltVSdct(luma, width*height, point);

    point = 16;
    luma = readBMP(lenaBmp, size_header, width, height);
    cout << "Lena, 16 point:" << endl;
    kltVSdct(luma, width*height, point);

    luma = readBMP(baboonBmp, size_header, width, height);
    cout << "Baboon, 16 point:" << endl;
    kltVSdct(luma, width*height, point);

    std::cout.rdbuf(coutbuf); //reset to standard output again

    return 0;
}
