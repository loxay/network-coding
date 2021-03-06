#include <jni.h>
#include <string>
#include "gf.c"

extern "C"{
jboolean ** Multiply(jboolean ** matA, jboolean ** matB, int nRow, int n, int nCol);
jboolean** Inverse(jboolean** G, int n);
JNIEXPORT jstring JNICALL
Java_com_example_android_nc_MainActivity_stringFromJNI(JNIEnv *env,jobject /* this */);
JNIEXPORT jobjectArray JNICALL
Java_com_example_android_nc_FileCut_Encode(JNIEnv *env, jobject instance,jbyteArray buffer_, jint N, jint K,jint nLen);
JNIEXPORT jobjectArray JNICALL
Java_com_example_android_nc_MainActivity_Encode(JNIEnv *env, jobject instance);
JNIEXPORT jboolean JNICALL
Java_com_example_android_nc_MainActivity_Testcode(JNIEnv *env, jobject instance);
JNIEXPORT jobjectArray JNICALL
Java_com_example_android_nc_FileCut_Decode(JNIEnv *env, jobject instance, jobjectArray buffer,
                                           jint nPart, jint nLength);
JNIEXPORT jobjectArray JNICALL
Java_com_example_android_nc_MainActivity_TestDecode(JNIEnv *env, jobject instance,
                                                    jobjectArray buffer, jint nPart, jint nLength);
JNIEXPORT jobjectArray JNICALL
Java_com_example_android_nc_FileCut_Reencode(JNIEnv *env, jobject instance,
                                             jobjectArray buffer, jint nPart,
                                             jint nLength);
}

//此处jbyte和c++中的byte预定义不一样
//c++中typedef unsigned char byte;
//jni中typedef unsigned char   jboolean;
//     typedef signed char     jbyte;
jboolean ** Multiply(jboolean** matA, jboolean ** matB, int nRow, int n, int nCol)
{
    int i, j, k;
    unsigned int temp;
    gf_init(8, 0x00000187);                //初始化域
    jboolean ** Mat;                            //用来存储矩阵相乘结果 (nRow*nCol)
    Mat = new jboolean*[nRow];
    for (i = 0; i < nRow; i++){
        Mat[i] = new jboolean[nCol];
    }

    for (i = 0; i < nRow; i++){            //两矩阵相乘
        for (j = 0; j < nCol; j++){
            temp = 0;
            for (k = 0; k < n; k++){
                temp = gf_add(temp, gf_mul(matA[i][k],matB[k][j]));
                //temp++;
            }
            Mat[i][j] = (jboolean)temp;
        }
    }
    gf_uninit();
    return Mat;
}

jboolean** Inverse(jboolean** G, int n){
    //求秩
    int i,j;
    int nRow = n;
    int	nCol = n;
    gf_init(8, 0x00000187);
    jboolean **M = new jboolean*[nRow];
    for (j = 0; j<nRow; j++)
    {
        M[j] = new jboolean[nCol];
    }
    for (i = 0; i<nRow; i++)
    {
        for (j = 0; j<nCol; j++)
        {
            M[i][j] = *(*(G + i) + j);
        }
    }
    // Define a variable to record the position of the main element.
    int yPos = 0;
    for (i = 0; i<nRow; i++)
    {
        // Find the main element which must be non-zero.
        bool bFind = false;
        for (int x = yPos; x<nCol; x++)
        {
            for (int k = i; k<nRow; k++)
            {
                if (M[k][x] != 0)
                {
                    // Exchange the two vectors.
                    for (int x = 0; x<nCol; x++)
                    {
                        jboolean nVal = M[i][x];
                        M[i][x] = M[k][x];
                        M[k][x] = nVal;
                    }										// We have exchanged the two vectors.	
                    bFind = true;
                    break;
                }
            }
            if (bFind==true)
            {
                yPos = x;
                break;
            }
        }
        for (j = i + 1; j<nRow; j++)
        {
            // Now, the main element must be nonsingular.
            unsigned int temp = gf_div(M[j][yPos], M[i][yPos]);
            for (int z = 0; z<nCol; z++)
            {
                M[j][z] = gf_add(M[j][z], gf_mul(temp, M[i][z]));
            }
        }
        yPos++;
    }
    // The matrix becomes a scalar matrix. we need to make more elements become 0 with elementary transformations.
    yPos = 0;
    for (i = 1; i<nRow; i++)
    {
        for (j = 0; j<nCol; j++)
        {
            if (M[i][j] != 0)
            {
                // the main element is found.
                yPos = j;
                break;
            }
        }
        for (int k = 0; k<i; k++)
        {
            unsigned int temp = gf_div(M[k][yPos], M[i][yPos]);
            for (int z = 0; z<nCol; z++)
            {
                M[k][z] = gf_add(M[k][z], gf_mul(temp, M[i][z]));
            }
        }
    }
    int nRank = 0;
    // Get the rank.
    for ( i = 0; i<nRow; i++)
    {
        int nNonzero = 0;
        for ( j = 0; j<nCol; j++)
        {
            if (M[i][j] != 0)
            {
                nNonzero++;
            }
        }
        // If there is only one nonzero element in the new matrix, it is concluded an original packet is leaked.
        if (nNonzero > 0)
        {
            // Leaked.
            nRank++;
        }
    }
    for ( i = 0; i<nRow; i++)
    {
        delete[]  M[i];
    }
    delete[] M;

    //求逆
    int bRet = nRank;
    if (bRet != nRow)
    {
        return NULL;
    }
    /************************************************************************/
    /**Start to get the inverse matrix!                                     */
    /************************************************************************/
    jboolean **N = new jboolean*[nCol];
    for ( j = 0; j<nCol; j++)
    {
        N[j] = new jboolean[2 * nCol];
    }
    for (i = 0; i<nCol; i++)
    {
        for (j = 0; j<nCol; j++)
        {
            N[i][j] = G[i][j];
        }
        for ( j = nCol; j<2 * nCol; j++)
        {
            if (i == j - nCol)
            {
                N[i][j] = 1;
            }
            else
            {
                N[i][j] = 0;
            }
        }
    }
    /************************************************************************/
    /** Step 1. Change to a lower triangle matrix.                           */
    /************************************************************************/
    for (i = 0; i<nCol; i++)
    {
        // There must exist a non-zero mainelement.
        if (N[i][i] == 0)
        {
            // Record this line.
            jboolean temp[200] = { 0 };
            for (int k = 0; k<2 * nCol; k++)
            {
                temp[k] = N[i][k];
            }
            // Exchange
            int nRow = nCol;					// They are the same in essensial.
            for (int z = i + 1; z<nRow; z++)
            {
                if (N[z][i] != 0)
                {
                    for (int x = 0; x<2 * nCol; x++)
                    {
                        N[i][x] = N[z][x];
                        N[z][x] = temp[x];
                    }
                    break;
                }
            }
        }

        for (j = i + 1; j<nCol; j++)
        {
            // Now, the main element must be nonsingular.
            unsigned int temp = gf_div(N[j][i], N[i][i]);
            for (int z = 0; z<2 * nCol; z++)
            {
                N[j][z] = gf_add(N[j][z], gf_mul(temp, N[i][z]));
            }
        }
    }
    /************************************************************************/
    /** Step 2. Only the elements on the diagonal are non-zero.                  */
    /************************************************************************/
    for (i = 1; i<nCol; i++)
    {
        for (int k = 0; k<i; k++)
        {
            unsigned int temp = gf_div(N[k][i], N[i][i]);
            for (int z = 0; z<2 * nCol; z++)
            {
                N[k][z] = gf_add(N[k][z], gf_mul(temp, N[i][z]));
            }
        }
    }
    /************************************************************************/
    /* Step 3. The elements on the diagonal are 1.                  */
    /************************************************************************/
    for (i = 0; i<nCol; i++)
    {
        if (N[i][i] != 1)
        {
            unsigned int temp = N[i][i];
            for (int z = 0; z<2 * nCol; z++)
            {
                N[i][z] = gf_div(N[i][z], temp);
            }
        }
    }
    /************************************************************************/
    /**Get the new matrix.                                                  */
    /************************************************************************/
    jboolean **CM = new jboolean*[nCol];
    for (j = 0; j<nCol; j++)
    {
        CM[j] = new jboolean[nCol];
    }
    for (i = 0; i<nCol; i++)
    {
        for (j = 0; j<nCol; j++)
        {
            CM[i][j] = N[i][j + nCol];
        }
    }
    // Clean the memory.
    gf_uninit();
    for (i = 0; i<nCol; i++)
    {
        delete[]  N[i];
    }
    delete[] N;
    return CM;
}



JNIEXPORT jstring JNICALL
Java_com_example_android_nc_MainActivity_stringFromJNI(JNIEnv *env,jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT jobjectArray JNICALL
Java_com_example_android_nc_FileCut_Encode(JNIEnv *env, jobject instance,
                                                            jbyteArray buffer_, jint N, jint K,jint nLen) {
    jbyte *buffer = env->GetByteArrayElements(buffer_, NULL);

    // TODO
    /************************************************************************/
    /* Step 1. Get matrix(K*nLen)                                         */
    /************************************************************************/
    int i,j;
    jboolean ** Buf;                                //文件数据存入二维数组中K*nLen
    Buf = new jboolean*[K];                        //把buffer数组存入二维数组Buf
    for (i = 0; i < K; i++){
        Buf[i] = new jboolean[nLen];
    }
   jbyte *p = buffer;            //用来遍历buffer数组
    for (i = 0; i < K; i++){
        for (j = 0; j < nLen; j++){
            Buf[i][j] = *p;
            p++;
        }
    }
    env->ReleaseByteArrayElements(buffer_, buffer, 0);

    /*jbyte ** Buf2;
    Buf2 = new jbyte*[K];                        //把buffer数组存入二维数组Buf
    for (i = 0; i < K; i++){
        Buf2[i] = new jbyte[nLen];
        for (j = 0; j < nLen; j++){
            Buf2[i][j] = Buf[i][j];
        }
    }*/
    /************************************************************************/
    /* Step 2. Get code matrix(N*K)                                         */
    /************************************************************************/
    jboolean ** encodeMatrix;          //编码矩阵N*K
   encodeMatrix = new jboolean*[N];
   for (i = 0; i < N; i++){
       encodeMatrix[i] = new jboolean[K];
   }

   srand((unsigned)time(NULL));
   for (i = 0; i < N; i++)      //生成随机矩阵
   {
       for (j = 0; j < K; j++)
       {
           encodeMatrix[i][j] = rand() % 256;
       }
   }

   /************************************************************************/
    /*Step 3. Start encoding                                               */
    /************************************************************************/
    jboolean ** matrix1;                             //用来存储编码结果N*nLen的矩阵
    matrix1 = Multiply(encodeMatrix, Buf, N, K, nLen);

    jbyte** Mat;                             //把编码矩阵与编码结果组成一个矩阵,这是一个N*(1+K+nLen)的矩阵
    Mat = new jbyte*[N];
    for (i = 0; i < N; i++){
        Mat[i] = new jbyte[1 + K + nLen];
    }
    for (i = 0; i < N; i++){                   //第0列全为K
        Mat[i][0] = K;
    }
    for (i = 0; i < N; i++){                   //1到K列为编码矩阵encodeMatrix
        for (j = 1; j <= K; j++){
            Mat[i][j] = encodeMatrix[i][j - 1];
        }
    }
    for (i = 0; i < N; i++){                   //K+1到K+nLen为编码结果
        for (j = K + 1; j <= K + nLen; j++){
           Mat[i][j] = matrix1[i][j - K - 1];
        }
    }
    int mWidth=1+K+nLen;
   // int mWidth=nLen;
    jobjectArray resultArray= env->NewObjectArray(N,env->FindClass("[B"),NULL);
    for (int i = 0; i < N; i++) {
        jbyteArray byteArray= env->NewByteArray(mWidth);
        env->SetByteArrayRegion(byteArray, 0, mWidth, Mat[i]);
       // env->SetByteArrayRegion(byteArray, 0, mWidth, Buf2[i]);
        env->SetObjectArrayElement(resultArray, i, byteArray);
        env->DeleteLocalRef(byteArray);
    }


    return resultArray;
}


//测试矩阵求逆
JNIEXPORT jobjectArray JNICALL
Java_com_example_android_nc_MainActivity_Encode(JNIEnv *env, jobject instance) {

    // TODO
    int mHeight=3,mWidth=3,i,j,x=0;
    jboolean ** out;
    out = new jboolean*[mHeight];
//    for (i = 0; i <mHeight ; i++){
//        out[i] = new jboolean[mWidth];
//        for(j=0;j<mWidth;j++) {
//                x+=20;
//                out[i][j] =x;
//        }
//    }
    for (i = 0; i <mHeight ; i++){
        out[i] = new jboolean[mWidth];
        for(j=0;j<mWidth;j++) {
            if (i == j)
                out[i][j] =2;
            else
                out[i][j] =0;
        }
    }
    out[2][1]=3;

//    jboolean ** out2;
//    out2 = new jboolean*[mHeight];
//    for (i = 0; i <mHeight ; i++){
//        out2[i] = new jboolean[mWidth];
//        for(j=0;j<mWidth;j++)
//            out2[i][j]=1;
//    }
    jboolean ** matrix1;                             //用来存储编码结果N*nLen的矩阵
//    matrix1 = Multiply( out, out2, 3, 3, 3);
    matrix1=Inverse(out,3);
    jbyte** Mat;
    Mat = new jbyte*[mHeight];
    for (i = 0; i <mHeight ; i++){
        Mat[i] = new jbyte[mWidth];
    }
    for (i = 0; i < mHeight; i++){
        for (j = 0; j <mWidth; j++){
            Mat[i][j] = matrix1[i][j];
        }
    }
    jobjectArray resultArray= env->NewObjectArray(mHeight,env->FindClass("[B"),NULL);
    for (int i = 0; i < mHeight; i++) {
        jbyteArray byteArray= env->NewByteArray(mWidth);
        env->SetByteArrayRegion(byteArray, 0, mWidth, Mat[i]);
        env->SetObjectArrayElement(resultArray, i, byteArray);
        env->DeleteLocalRef(byteArray);
    }

    return resultArray;

}

JNIEXPORT jboolean JNICALL
Java_com_example_android_nc_MainActivity_Testcode(JNIEnv *env, jobject instance) {

    // TODO

    jboolean temp=2;
    gf_init(8, 0x00000187);                //初始化域

    temp = gf_add(temp, gf_mul(1, 233));

    gf_uninit();
    return temp;
}

JNIEXPORT jobjectArray JNICALL
Java_com_example_android_nc_FileCut_Decode(JNIEnv *env, jobject instance, jobjectArray buffer,
                                           jint nPart, jint nLength) {

    // TODO
    /**接收二维数组参数转为jboolean类型*/
    jboolean ** MAT= new jboolean *[nPart];
    int i,j;
    for (i = 0; i < nPart; i++) {
        MAT[i] = new jboolean[nLength];
        jbooleanArray bytedata = (jbooleanArray) env->GetObjectArrayElement(buffer, i);
        MAT[i] = env->GetBooleanArrayElements(bytedata, 0);
        //env->GetBooleanArrayRegion(bytedata,0,nLength,(jboolean *)&MAT[i]);
        env->DeleteLocalRef(bytedata);//释放内存，防止内存泄漏
    }

    /**从数组中分离编码矩阵和编码结果*/
    jboolean** encodeMAT;                                     //编码矩阵为nPart*nPart矩阵
    encodeMAT = new jboolean*[nPart];
    for (i = 0; i < nPart; i++){
        encodeMAT[i] = new jboolean[nPart];
    }
    for (i = 0; i < nPart; i++){
        for (j = 0; j < nPart; j++){
            encodeMAT[i][j] = MAT[i][j + 1];                  //赋值
        }
    }
    jboolean** MAT1;                                          //编码结果为nPart*(nLength-nPart-1)矩阵
    MAT1 = new jboolean*[nPart];
    for (i = 0; i < nPart; i++)
    {
        MAT1[i] = new jboolean[nLength - nPart - 1];
    }
    for (i = 0; i < nPart; i++)
    {
        for (j = 0; j < nLength - nPart - 1; j++)
        {
            MAT1[i][j] = MAT[i][j + 1 + nPart];     //赋值
        }
    }

    /**对编码矩阵求逆*/
    jboolean** IvEncodeMAT;
    IvEncodeMAT = Inverse(encodeMAT, nPart);
    if (IvEncodeMAT==NULL){
        return NULL;
    }
    int mWidth=nLength-nPart-1;          //矩阵列数
    jboolean** dataMat;                  //用来存储编码结果nPart*(nLength - nPart - 1)的矩阵
    dataMat = Multiply(IvEncodeMAT, MAT1, nPart, nPart, mWidth);

//    int mWidth=nLength;
    jbyte** Mat;
    Mat = new jbyte*[nPart];
    for (i = 0; i <nPart ; i++){
        Mat[i] = new jbyte[mWidth];
    }
    for (i = 0; i < nPart; i++){
        for (j = 0; j <mWidth; j++){
            Mat[i][j] = dataMat[i][j];
//            Mat[i][j] = MAT[i][j];
        }
    }
    jobjectArray resultArray= env->NewObjectArray(nPart,env->FindClass("[B"),NULL);
    for (int i = 0; i < nPart; i++) {
        jbyteArray byteArray= env->NewByteArray(mWidth);
        env->SetByteArrayRegion(byteArray, 0, mWidth, Mat[i]);
        env->SetObjectArrayElement(resultArray, i, byteArray);
        env->DeleteLocalRef(byteArray);
    }
    return resultArray;
}

JNIEXPORT jobjectArray JNICALL
Java_com_example_android_nc_MainActivity_TestDecode(JNIEnv *env, jobject instance,
                                                    jobjectArray buffer, jint nPart, jint nLength) {

    // TODO
    /**接收二维数组参数转为jboolean类型*/
    jboolean ** MAT= new jboolean *[nPart];
    int i,j;
    for (i = 0; i < nPart; i++) {
        MAT[i] = new jboolean[nLength];
        jbooleanArray bytedata = (jbooleanArray) env->GetObjectArrayElement(buffer, i);
        MAT[i] = env->GetBooleanArrayElements(bytedata, 0);
        //env->GetBooleanArrayRegion(bytedata,0,nLength,(jboolean *)&MAT[i]);
        env->DeleteLocalRef(bytedata);//释放内存，防止内存泄漏
    }



    jbyte** Mat;
    Mat = new jbyte*[nPart];
    for (i = 0; i <nPart ; i++){
        Mat[i] = new jbyte[nLength];
    }
    for (i = 0; i < nPart; i++){                   //1到K列为编码矩阵encodeMatrix
        for (j = 0; j <nLength; j++){
            Mat[i][j] = MAT[i][j];
        }
    }
    jobjectArray resultArray= env->NewObjectArray(nPart,env->FindClass("[B"),NULL);
    for (int i = 0; i < nPart; i++) {
        jbyteArray byteArray= env->NewByteArray(nLength);
        env->SetByteArrayRegion(byteArray, 0, nLength, Mat[i]);
        env->SetObjectArrayElement(resultArray, i, byteArray);
        env->DeleteLocalRef(byteArray);
    }
      return resultArray;

    //return  1;
}

JNIEXPORT jobjectArray JNICALL
Java_com_example_android_nc_FileCut_Reencode(JNIEnv *env, jobject instance,
                                                                    jobjectArray buffer, jint nPart,
                                                                    jint nLength) {

    // TODO
    /**接收二维数组参数转为jboolean类型*/
    jboolean ** Buffer= new jboolean *[nPart];
    int i,j;
    for (i = 0; i < nPart; i++) {
        Buffer[i] = new jboolean[nLength];
        jbooleanArray bytedata = (jbooleanArray) env->GetObjectArrayElement(buffer, i);
        Buffer[i] = env->GetBooleanArrayElements(bytedata, 0);
        //env->GetBooleanArrayRegion(bytedata,0,nLength,(jboolean *)&MAT[i]);
        env->DeleteLocalRef(bytedata);//释放内存，防止内存泄漏
    }

    //取出编码矩阵和编码后的数据
    jboolean** matrix1;                             //编码矩阵为nPart*(nLength-1)矩阵
    matrix1 = new jboolean*[nPart];
    for (i = 0; i < nPart; i++){
        matrix1[i] = new jboolean[nLength-1];
    }
    for (i = 0; i < nPart; i++){
        for (j = 0; j < nLength-1; j++){
            matrix1[i][j] = Buffer[i][j + 1];         //Buffer数组的第0列存的是文件编码前平均分成的个数，这一列不读出
        }
    }

    //生成随机的再编码矩阵
    jboolean ** re_encodeMatrix;
    re_encodeMatrix = new jboolean*[nPart];
    for (i = 0; i < nPart; i++){
        re_encodeMatrix[i] = new jboolean[nPart];
    }
    srand((unsigned)time(NULL));
    for (i = 0; i < nPart; i++)
    {
        for (j = 0; j < nPart; j++)
        {
            re_encodeMatrix[i][j] = rand() % 256;
        }
    }


    jboolean ** MAT;                                 //再编码
    MAT = Multiply(re_encodeMatrix, matrix1, nPart, nPart, nLength-1);

    for (i = 0; i < nPart; i++){                   //把再编码后的数据重新读回数组
        for (j = 1; j < nLength; j++){
            Buffer[i][j] = MAT[i][j - 1];
        }
    }

    int mWidth=nLength;
    jbyte** Mat;
    Mat = new jbyte*[nPart];
    for (i = 0; i <nPart ; i++){
        Mat[i] = new jbyte[mWidth];
    }
    for (i = 0; i < nPart; i++){
        for (j = 0; j <mWidth; j++){
            Mat[i][j] = Buffer[i][j];
        }
    }
    jobjectArray resultArray= env->NewObjectArray(nPart,env->FindClass("[B"),NULL);
    for (int i = 0; i < nPart; i++) {
        jbyteArray byteArray= env->NewByteArray(mWidth);
        env->SetByteArrayRegion(byteArray, 0, mWidth, Mat[i]);
        env->SetObjectArrayElement(resultArray, i, byteArray);
        env->DeleteLocalRef(byteArray);
    }
    return resultArray;

}