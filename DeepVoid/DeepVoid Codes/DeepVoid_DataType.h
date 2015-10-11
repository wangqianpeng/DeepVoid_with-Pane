/************************************************************************************/
/* Johnny Zhu's own library, including kinds of self-defined data types and classes.*/
/* Author: Johnny Zhu (DeepVoid) in Melbourne                        Date:2014.03.02*/
/************************************************************************************/

#pragma once

// #include "opencv/cv.h"
// #include "opencv/highgui.h"
// #include "opencv/cvaux.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/nonfree.hpp"
using namespace std;
using namespace cv;

namespace DeepVoid
{

#define	FTOI(a)			( (a) < 0 ? ((int)(a-0.5)) : ((int)(a+0.5)) )

const double R2D = 180/CV_PI;
const double D2R = CV_PI/180.0;

// zhaokunz, 20140406, fast floor and ceil, downloaded from http://www.codeproject.com/Tips/700780/Fast-floor-ceiling-functions
inline int floor_fast(double fp)
{
	return int(fp + 65536.) - 65536;
}
 
inline int ceil_fast(double fp)
{
	return 65536 - int(65536. - fp);
}
 
inline int floor_fast_noshift(double fp)
{
	int i = int(fp); 
	return i > fp ? i - 1 : i;
}
 

inline int ceil_fast_noshift(double fp)
{
	int i = int(fp);
	return i < fp ? i + 1 : i;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#define MINSCALENUM			0	// the index of the minimal scale
#define MAXSCALENUM			13	// the index of the maximal scale
#define FITSCALENUM			6

const float ImageDisplayScales[] = {0.03125, 0.0625, 0.125, 0.25, 0.5, 0.75, 1, 1.5, 2, 3, 5, 10, 20, 25};

// zhaokunz, 20140226
enum ChildFrmLayoutType
{
	ChildFrm_Cascade       = 0,     
	ChildFrm_Tabbed        = 1,
	ChildFrm_TileVert      = 2,
	ChildFrm_TileHorz      = 3,
};

BOOL DisplayImage(CDC * pDC,									// pointer to CDC
				  const cv::Mat & img,							// image to be displayed
				  int tl_x, int tl_y, int rect_w, int rect_h	// region to put the image in
				  );

CString GetFileNameOnly(CString filePath);

enum FeatureType
{
	Feature_SIFT       = 0,     
	Feature_SURF       = 1,
};

struct Features
{
	std::vector<cv::KeyPoint> key_points;
	cv::Mat descriptors;
	std::vector<int> tracks;

	// zhaokunz, 20140324, sometimes multiple keypoints correspond to the same image point
	// like the SIFT features, a feature can have multiple orientations
	// this idx indicates the unique index of this keypoint, if two keypoints correspond to the same image point
	// their indices will be the same, otherwise will be different
	std::vector<int> idx_pt;

	// 20150214, zhaokunz, ���������㴦ͨ��˫���Բ�ֵ�õ��ĵ���ɫ��Ϣ
	std::vector<cv::Vec3b> rgbs;

	FeatureType type;

	Features()
	{
		type = Feature_SIFT;
	};

	Features & operator = (const Features & otherFeat)
	{
		if (this != &otherFeat)
		{
			type = otherFeat.type;
			key_points = otherFeat.key_points;
			descriptors = otherFeat.descriptors.clone();
			tracks = otherFeat.tracks;
			idx_pt = otherFeat.idx_pt;
			rgbs = otherFeat.rgbs;
		}

		return *this;
	};

	Features(const Features & feat)
	{
		*this = feat;
	};
};

struct CloudPoint_ImgInfo
{
	int m_idxImg;					// indicates in which image is this cloud point observed
	int m_idxImgPt;					// indicates which image point does this cloud point correspond to in this image
	Vec3b m_rgb;					// what is the RGB value of this cloud point in this image
	double m_rpjErr;				// what is the reprojection error of this cloud point in this image

	CloudPoint_ImgInfo()
	{
		m_idxImg = m_idxImgPt = -1;
		m_rgb.val[0] = m_rgb.val[1] = m_rgb.val[2] = 0;
		m_rpjErr = 0;
	};

	CloudPoint_ImgInfo & operator = (const CloudPoint_ImgInfo & otherInfo)
	{
		if (this != &otherInfo)
		{
			m_idxImg = otherInfo.m_idxImg;
			m_idxImgPt = otherInfo.m_idxImgPt;
			m_rgb = otherInfo.m_rgb;
			m_rpjErr = otherInfo.m_rpjErr;
		}

		return *this;
	};

	// 20150124, zhaokunz, override the == operator, so that functions like find(first, end, value) can be used to determine equality
	bool operator == (const CloudPoint_ImgInfo & otherInfo)
	{
		// the imgInfo equals the other one only if they are from the same image and the same image point
		return (m_idxImg == otherInfo.m_idxImg && m_idxImgPt == otherInfo.m_idxImgPt);
	};

	CloudPoint_ImgInfo(const CloudPoint_ImgInfo & info)
	{
		*this = info;
	};
};

struct CloudPoint
{
	int m_idx;									// the global index of this cloud point in the whole point cloud
	Point3d m_pt;								// the 3d coordinates of this cloud point
	vector<CloudPoint_ImgInfo> m_vImgInfos;		// all the image infos of this cloud point in all visible images

	CloudPoint()
	{
		m_idx = -1;
		m_pt.x = m_pt.y = m_pt.z = 0;
	};

	CloudPoint & operator = (const CloudPoint & otherPt)
	{
		if (this != &otherPt)
		{
			m_idx = otherPt.m_idx;
			m_pt = otherPt.m_pt;
			m_vImgInfos = otherPt.m_vImgInfos;
		}

		return *this;
	};

	CloudPoint(const CloudPoint & pt)
	{
		*this = pt;
	};
};

struct cam_data
{
	double fx; double fy;   // equivalent focal length
	double cx; double cy;   // principal point
	double s;	            // skew factor

	double R[9];            // rotation matrix
	double t[3];            // translation vector

	double k[5];            // distortion

	// 20150214, zhaokunz, ����Matx�ṹ��
	Matx33d m_K;
	Matx33d m_R;
	Matx31d m_t;

	bool m_bCalibed;	// whether interior calibrated or not
	bool m_bOriented;	// whether exterior oriented or not
	//////////////////////////////////////////////////////////////////////////
	
	int dist_type;          // distortion type, 0 is Weng's, 1 is D.C.Brown's

	Features m_feats;		// image features

	cam_data()
	{
		fx = -1;	fy = -1;
		cx = -1;	cy = -1;
		s  = -1;

		int i;
		for (i = 0; i < 9; i++)
		{
			R[i] = -100;
		}

		for (i = 0; i < 3; i++)
		{
			t[i] = -1;
		}

		for (i = 0; i < 5; i++)
		{
			k[i] = -1;
		}

		m_bCalibed = false;
		m_bOriented = false;
	};

	cam_data & operator = (const cam_data & otherCam)
	{
		if (this != &otherCam)
		{
			fx = otherCam.fx; fy = otherCam.fy;
			cx = otherCam.cx; cy = otherCam.cy;
			s  = otherCam.s;

			memcpy(R, otherCam.R, 9*sizeof(double));
			memcpy(t, otherCam.t, 3*sizeof(double));
			memcpy(k, otherCam.k, 5*sizeof(double));

			dist_type = otherCam.dist_type;

			m_feats = otherCam.m_feats;

			m_K = otherCam.m_K;
			m_R = otherCam.m_R;
			m_t = otherCam.m_t;
			m_bCalibed = otherCam.m_bCalibed;
			m_bOriented = otherCam.m_bOriented;
		}

		return *this;
	};

	cam_data(const cam_data & cam)
	{
		*this = cam;
	};
};

// ����һ��������ת˳���ö������
enum RotationOrder
{
	XYZ,
	XZY,
	YXZ,
	YZX,
	ZXY,
	ZYX
};

// ����Ƕȵ�cosֵ������Ϊ�Ƕ�
double cosd(double angle);

// ����Ƕȵ�sinֵ������Ϊ�Ƕ�
double sind(double angle);

// ����Ƕȵ�tanֵ������Ϊ�Ƕ�
double tand(double angle);

// ����������ֵ��arccosineֵ������ֵ��ΧΪ0 - 180��
double acosd(double x);

// ����������ֵ��arcsineֵ������ֵ��ΧΪ-90 - 90��
double asind(double x);

// ����������ֵ��arctangentֵ������ֵ��ΧΪ-90 - 90��
double atand(double x);

// ����(y / x)��arctangentֵ�����ڿ����ۺϿ���y��x�ķ��ţ���˷���ֵ�ķ�Χ��չ���ĸ����ޣ���-180 - 180��
double atand(double y, double x);

// �ж����� x �ķ��ţ�Ϊ���򷵻� 1��Ϊ���򷵻� -1
double Sign_Johnny(double x);

// Implementation of bilinear interpolation
double BilinearInterp(double x, double y,	// the coordinates of position to be interpolated
					  double x1,			// the x coordinate of topleft f1 and bottomleft f3
					  double x2,			// the x coordinate of topright f2 and bottomright f4
					  double y1,			// the y coordinate of topleft f1 and topright f2
					  double y2,			// the y coordinate of bottomleft f3 and bottomright f4
					  double f1, double f2, double f3, double f4	// the corresponding values of topleft,topright,bottomleft and bottomright points
					  );

bool BilinearInterp(const Mat & img,		// input:	the image data
					double x, double y,		// input:	the coordinates of position to be interpolated
					uchar & r,				// output:	the interpolated R
					uchar & g,				// output:	the interpolated G
					uchar & b				// output:	the interpolated B
					);

bool BilinearInterp(const Mat & img,		// input:	the image data
					double x, double y,		// input:	the coordinates of position to be interpolated
					double & r,				// output:	the interpolated R
					double & g,				// output:	the interpolated G
					double & b				// output:	the interpolated B
					);

bool BilinearInterp(const Matx33d & mK,				// input:	the camera matrix
					const Matx33d & mR,				// input:	the rotation matrix
					const Matx31d & mt,				// input:	the translation vector
					const Mat & img,				// input:	the input image
					double X, double Y, double Z,	// input:	the coordinates of position to be interpolated
					uchar & r,						// output:	the interpolated R
					uchar & g,						// output:	the interpolated G
					uchar & b						// output:	the interpolated B
					);

bool BilinearInterp(const Matx33d & mK,				// input:	the camera matrix
					const Matx33d & mR,				// input:	the rotation matrix
					const Matx31d & mt,				// input:	the translation vector
					const Mat & img,				// input:	the input image
					double X, double Y, double Z,	// input:	the coordinates of position to be interpolated
					double & r,						// output:	the interpolated R
					double & g,						// output:	the interpolated G
					double & b,						// output:	the interpolated B
					double * imgpt_x = NULL,
					double * imgpt_y = NULL
					);

void MakeSureNotOutBorder(int x, int y,				// input:	original center of rect
	                      int & x_new, int & y_new,	// output:	new center of rect, making sure the new rect with the same size are within border
						  int wndSizeHalf,
						  int imgWidth, int imgHeight
						  );

// CvMat wrapper here /////////////////////////////////////////////////////////////////////////////////

#define EPSILON_ZHU		1.110223E-16

// ������CMatrix
// ����������������ü������ƣ�����˺�������Ч�ʣ���ͬʱҲ���������ɶ��Ե��½�
// ���磺CMatrix m1, m2; m1 = m2 �����ǽ�m2�����������ݸ��Ƶ�m1��������������m1��m2�е�m_pMatָ��ͬһ������ͷ�����m1��m2Ҳ��������ͬһ�����������ڸı�m1����
// ĳԪ��ֵ��ͬʱ��m2��ӦԪ�ص�ֵҲ�����˸ı䣬��m1 = m2 ��仰��ʹ��������������ȫ��������������ٶ�����CMatrix m1 = m2Ч����һ���ġ�
// ������������ȫ�����ľ��󣨼����������и��Զ�����������������������һ����������ݸ��Ƶ���һ��������������Ļ�������ͨ����� CMatrix m1 = m2.Clone()��

// �������תģʽ��ö������
enum FlipMode
{
	FLIPMODE_X,
	FLIPMODE_Y,
	FLIPMODE_XY
};

enum MosaicMode
{
	MOSAICMODE_DOWN,
	MOSAICMODE_UP,
	MOSAICMODE_RIGHT,
	MOSAICMODE_LEFT
};

enum InverseMode
{
	INVERSEMODE_LU,
	INVERSEMODE_SVD,
	INVERSEMODE_SVD_SYM
};

enum SolveMode
{
	SOLVEMODE_LU,
	SOLVEMODE_SVD,
	SOLVEMODE_SVD_SYM
};

enum RandMode
{
	RANDMODE_UNIFORM,  // Uniform distribution
	RANDMODE_GAUSSIAN  // Gaussian distribution
};

enum NormMode
{
	NORMMODE_C,
	NORMMODE_L1,
	NORMMODE_L2
};

enum WriteMode
{
	WRITEMODE_LF,
	WRITEMODE_E
};

class CMatrix
{
public:
	CMatrix();
	CMatrix(int nRow, int nColumn, double initVal = 0);
	CMatrix(CvMat * pCvMat);
	CMatrix(const CMatrix & mat);                                     // �������캯�������ڴ����Լ����������
	/*CMatrix(int nRow, int nColumn, double * pInitArray);*/
	virtual ~CMatrix();

	// ����
public:                                                
	int     m_nRow;                                                   // ��������
	int     m_nCol;                                                   // ��������
	CvMat * m_pMat;                                                   // ��װ��OpenCV�ľ���ṹ���ָ��

private:
	/*CvMat * m_pMat;                                                   // ��װ��OpenCV�ľ���ṹ���ָ��*/

	// ����
public:
	double & operator () (int i, int j) const;                        // ������ʽ��1��ʼ
	double & operator () (int i)        const;                        // ��������������ʽ�����������������������������ͨ��һά������ʽ�Ծ���Ԫ�ؽ���ȡֵ��ֵ

	CMatrix & operator = (const CMatrix & mSource);                   // ���ظ�ֵ�������ʹ�ñ������mSource�е�m_pMatָ��ͬһ������ͷ�����Ҳӵ��ͬһ�����ü����洢�������������ı䱾��������ݻ�ʹ��mSource������Ҳ�����仯
	CMatrix & operator = (double val);                                // �Ѿ�������Ԫ�ص�ֵ�û�Ϊval

	CMatrix operator + (const CMatrix & mAdd) const;                  // ���ؼӺ������
	CMatrix operator + (double val) const;                            // ��������Ԫ�ص�ֵ��val
	friend CMatrix operator + (double val, const CMatrix & mAdd);     // ��������Ԫ�ص�ֵ���һ��val

	CMatrix operator - (const CMatrix & mSub) const;				  // ���ؼ��������
	CMatrix operator - (double val) const;                            // ��������Ԫ�ص�ֵ��val
	CMatrix operator - () const;                                      // ��������Ԫ��ȡ������Ŀ���������
	friend CMatrix operator - (double val, const CMatrix & mSub);	  // ��������Ԫ�ص�ֵ���һ��val

	CMatrix operator * (const CMatrix & mMul) const;				  // ���س˷������
	CMatrix operator * (double val) const;                            // �ҳ�һ����
	friend CMatrix operator * (double val, const CMatrix & mMul);     // ���һ����

	CMatrix operator / (const CMatrix & mDiv) const;                  // �������һ����ͬ�ߴ�ľ������������Ԫ�ض�Ӧ���������һ���������һ�������
	CMatrix operator / (double val) const;                            // �������Ԫ�س���val
	friend CMatrix operator / (double val, const CMatrix & mDiv);     // һ��������һ������

	void operator += (const CMatrix & mAdd);                          // �����Լ�һ����
	void operator += (double val);                                    // �����Լ�һ��

	void operator -= (const CMatrix & mSub);                          // �����Լ�һ����
	void operator -= (double val);                                    // �����Լ�һ��

	void operator *= (const CMatrix & mMul);                          // �����Գ�һ������ָ����ͬ�ߴ�������Ԫ�ض�Ӧ���
	void operator *= (double val);                                    // �����Գ�һ��

	void operator /= (const CMatrix & mDiv);                          // �����Գ�һ������ָ����ͬ�ߴ�������Ԫ�ض�Ӧ���
	void operator /= (double val);                                    // �����Գ�һ��



	void Release(void);                                               // �������ü������Ƶľ����ͷź���



	BOOL IsNull(void)       const;                                    // �Ƿ��ǿվ��󣬼�m_pMat�Ƿ�ΪNULL
	BOOL IsZero(void)       const;                                    // �Ƿ��������
	BOOL IsVector(void)     const;                                    // �Ƿ�������
	BOOL IsRowVec(void)     const;                                    // �Ƿ���������
	BOOL IsColVec(void)     const;                                    // �Ƿ���������
	BOOL IsSquare(void)     const;                                    // �Ƿ��Ƿ���
	BOOL IsSymmetric(void)  const;                                    // �Ƿ��ǶԳ���
	BOOL IsAntiSymm(void)   const;                                    // �Ƿ��Ƿ��Գ���
	BOOL IsDiagonal(void)   const;                                    // �Ƿ��ǶԽ��󣬲�һ����Ҫ�Ƿ���
	BOOL IsIdentity(void)   const;                                    // �Ƿ��ǵ�λ��
	BOOL IsSymmPosDef(void) const;                                    // �Ƿ��ǶԳ���������������Cholesky�ֽ�



	CMatrix Clone(void)     const;                                    // ���صĶ����е�m_pMatָ��һ���µľ���ͷ����������һ���µ������������µ��������е������ǶԱ��������������������ݵĿ���
	CMatrix GetRow(int i)   const;                                    // ���ؾ�������ĳһ�У�ҲΪCMatrix����������ʽ��1��ʼ
	CMatrix GetCol(int i)   const;                                    // ���ؾ�������ĳһ�У�ҲΪCMatrix����������ʽ��1��ʼ
	CMatrix GetRect(int rowTL, int colTL, int width, int height) const; // ���ؾ�������ĳһ���ο飬ҲΪһ�������������ʽ��1��ʼ
	CMatrix GetDiag(void)   const;                                    // ���ؾ���Խ����ϵ�Ԫ��
	CMatrix Transpose(void) const;                                    // ����ת��
	CMatrix Inverse(void)   const;                                    // ��������
	CMatrix Pow(double power) const;                                  // ���еľ���Ԫ������
	CMatrix Log(void)       const;                                    // ���еľ���Ԫ��ȡ����
	CMatrix Max(void)       const;                                    // �ҵ������ȫ�����ֵ�������������ؾ���ĵ�һ��Ϊ�������ֵ����Ԫ�ص����������ڶ���Ϊ����������������Ϊ�����ֵ
	CMatrix Min(void)       const;                                    // �ҵ������ȫ����Сֵ�������������ؾ���ĵ�һ��Ϊ������Сֵ����Ԫ�ص����������ڶ���Ϊ����������������Ϊ����Сֵ
	CMatrix Find(double val, double eps = 1.0E-10) const;             // �ھ������ҵ�ֵΪval��Ԫ�أ�������ЩԪ�ص����������ţ����ؾ���ĵ�һ��Ϊ���������ڶ���Ϊ������
	CMatrix Abs(void)       const;                                    // ���صľ���������Ԫ��Ϊԭ�����ӦԪ�صľ���ֵ
	CMatrix Atand(void)     const;                                    // ���صľ����е�Ԫ��Ϊԭ�����ж�ӦԪ�صķ�����ֵ



	void SetRow(int i, const CMatrix & mSrc);                         // �����е�һ������mSrc��ֵ������ĵ�i�У�������ʽ��1��ʼ
	void SetCol(int i, const CMatrix & mSrc);                         // �����е�һ������mSrc��ֵ������ĵ�i�У�������ʽ��1��ʼ
	void SetRect(int rowTL, int colTL, const CMatrix & mSrc);         // �����е�һ������mSrc��ֵ�������ĳһ��������򣬱���ֵ�ľ��������С��mSrc�ĳߴ����
	void AddOneRow(double initVal = 0);                               // ��ԭ��������һ�к�������һ���γ��µ���չ�ľ���
	void AddOneCol(double initVal = 0);                               // ��ԭ��������һ�к�������һ���γ��µ���չ�ľ���
	void Flip(FlipMode flipMode);                                     // ��������ķ�תģʽ���þ�����x�ᷭת������y�ᷭת������Ǽ���x������y�ᷭת
	void Mosaic(const CMatrix & mOther, MosaicMode mode = MOSAICMODE_DOWN); // ����һ������һ��ģʽƴ�ӵ���������
	void Reshape(int newRow, int newCol);                             // �ı����ĳߴ磬������Ԫ�ص�������Ȼ����
	void VectorizeByRow(void);                                        // �����������г�ʸ����ʽ
	void VectorizeByCol(void);                                        // �����������г�ʸ����ʽ



	double Det(void)                         const;				      // �����������ʽ��ֵ
	double Trace(void)                       const;                   // �������ļ�
	int    Rank(double eps = 1.0E-10)        const;                   // ����������
	double InvCond(void)                     const;                   // ��������������ĵ���������С����ֵ���������ֵ�ı�ֵ
	double Mean(void)                        const;                   // �����������Ԫ�ص�ƽ��ֵ
	double StdDev(void)                      const;                   // �����������Ԫ�صı�׼��
	int    CountNonZero(void)                const;                   // ͳ�ƾ������Ԫ�صĸ���
	double Sum(void)                         const;                   // �Ծ���������Ԫ�����
	double Norm(NormMode mode = NORMMODE_L2) const;                   // �������ķ���
	int    Length(void)                      const;                   // ��CMatrix�����������򷵻��������ȣ���Ϊ�����򷵻�ӵ��Ԫ�������Ǹ�ά���ϵ�Ԫ�ظ���
	double Dev(void)                         const;                   // ���� sqrt(sum(m(i,j)^2) / n)



	friend CMatrix Mul(const CMatrix & m1, const CMatrix & m2);       // ��ͬ��С�ľ������Ԫ�ض�Ӧ���
	friend CMatrix Div(const CMatrix & m1, const CMatrix & m2);       // ��ͬ��С�ľ������Ԫ�ض�Ӧ���
	friend CMatrix ExtendRows(const CMatrix & mMat, int row);         // ������ľ���row�н�����չ
	friend CMatrix ExtendCols(const CMatrix & mMat, int col);         // ������ľ���col�н�����չ
	friend CMatrix RepMat    (const CMatrix & mMat, int row, int col);// ������ľ���row�к�col�н�����չ
	friend CMatrix Rand(int row, int col, RandMode randMode, double param1, double param2); // �������������ɵľ���randModeΪ���ֲַ����������param1��param2��Ϊuniform�ֲ�ʱ�ֱ��ʾ���½磬��gaussianʱ�ֱ�Ϊ�������ƽ��ֵ�ͱ�׼��
	friend CMatrix Solve(const CMatrix & mA, const CMatrix & mB);     // ���Ax=B
	friend CMatrix Cross(const CMatrix & mVec1, const CMatrix & mVec2);// �������ά�����Ĳ�ˣ�mVec1��mVec2
	friend void    RQ3x3(const CMatrix & mSrc, CMatrix & mR, CMatrix & mQ);// ��3��3�ľ������RQ�ֽ⣺A = RQ������RΪ�Խ���Ԫ��ȫΪ���������Ǿ���Q��Ϊ��������
	friend CMatrix GenI(int n);                                       // ����n��n�ĵ�λ��
	friend CMatrix GenCrossMat(const CMatrix & mVec);                 // �����������ά����v�������Ӧ�Ĳ�˾���[v]
	friend void    House(const CMatrix & mX, CMatrix & mVec, double & beta); // �������������mX��������Ӧ��Householder vector mVec���Լ���
	friend void    Givens(double a, double b, double & c, double & s);// ���������[a; b]'����ʹ��[c s; -s c]' * [a; b]' = [r; 0]'������cos��sin



	void    QR_Householder(CMatrix & mR, CMatrix & mQ = CMatrix(0)) const;  // ��m��n�ľ���A����QR�ֽ⣺A = QR������QΪm��m����������RΪm��n�ģ������Խ���Ԫ�ؽ�Ϊ���������Ǿ���
	void    QR_Givens(CMatrix & mR, CMatrix & mQ = CMatrix(0)) const;       // ��m��n�ľ���A����QR�ֽ⣺A = QR������QΪm��m����������RΪm��n�ģ������Խ���Ԫ�ؽ�Ϊ���������Ǿ���
	void    RQ_Givens(CMatrix & mR, CMatrix & mQ = CMatrix(0)) const;       // ��m��n�ľ���A����RQ�ֽ⣨Ҫ��m<=n����A = RQ������QΪn��n����������RΪm��n�ģ������Խ���Ԫ�ؽ�Ϊ���������Ǿ���
	CMatrix Chol_Lower(double eps = EPSILON_ZHU)   const;                   // �ԶԳ���������ʵʩCholesky�ֽ� A = LL'�����ضԽ�����Ԫ��ȫΪ���������Ǿ���L���˺���Ҳ�����ж϶Գƾ����Ƿ��������ֽ�ɹ��򷵻صľ������m_pMat��ΪNULL������ΪNULL
	CMatrix Chol_Upper(double eps = EPSILON_ZHU)   const;                   // �ԶԳ���������ʵʩCholesky�ֽ� A = UU'�����ضԽ�����Ԫ��ȫΪ���������Ǿ���U���˺���Ҳ�����ж϶Գƾ����Ƿ��������ֽ�ɹ��򷵻صľ������m_pMat��ΪNULL������ΪNULL
	void    SVD(CMatrix & mD, CMatrix & mU = CMatrix(0), CMatrix & mV = CMatrix(0)) const; // �Ծ���ʵʩSVD�ֽ⣬A = UDV'



	void Write(CString filePath, WriteMode mode = WRITEMODE_LF) const;      // �Ѿ���д��ָ���ļ�
	BOOL Read(CString filePath);                                            // ���ļ��������ݵ��������
	void PrintMatrix2Screen(CString title = "", WriteMode mode = WRITEMODE_LF) const;     // ͨ��AfxMessageBox�Ѿ�����ʾ����Ļ��
};
///////////////////////////////////////////////////////////////////////////////////////////////////////

// levmar related structures ////////////////////////////////////////////////////////////////////////
struct levmar_mImgPts1_mImgPts2_bx
{
	CMatrix mImgPts1;
	CMatrix mImgPts2;
	double bx;
};

struct levmar_calib_data
{
	cam_data camData;
	vector<Point3d> objPts;
};
///////////////////////////////////////////////////////////////////////////////////////////////////

// std related ////////////////////////////////////////////////////////////////////////////////////

// Return whether first element is greater than the second
bool Greater_DMatch_queryIdx(DMatch match1, DMatch match2);
bool Greater_DMatch_trainIdx(DMatch match1, DMatch match2);

///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���ܵĽⷽ�̼����������� /////////////////////////////////////////////////////////////////////////////////
bool EquSlvPdI_611(double *M, double *B, int m, int n, double *S);
bool MtxInvSP_611(double *A, int n);
bool MtxPdI_611(double *A, int m, int n);
void MtxMlt_611(double *A, double *B, int m, int n, int s, double *AB);
void MtxTrs_611(double *A, int m, int n, double *AT);
void ATAMlt_611(double *A, int m, int n, double *ATA);

bool M_Inversion_611(double *MatI, double *MatO, int n);
int M_DecompSVD_611(double *a,int m,int n,double *u,double *v,double eps,int ka);
void ppp_611(double *a,double *e,double *s,double *v,int m,int n);
void sss_611(double *fg,double *cs);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// �� 1 ά�������ð�ݷ����򣬿�����������Ҳ�ɽ������У������ԭ�����Ԫ����������������������е�����
void Sort_Bubble(double * p,			// ��������������������飬����ִ����������е�Ԫ�ؽ���������߽�������
				 int n,					// ���룺���鳤�ȣ�Ԫ�ظ���
				 int * idx,				// �����������ɺ��Ԫ��ԭ���������ŵ���������������� 0 ��ʼ
				 BOOL bIsAscent = TRUE	// ���룺�����򣬻��ǽ�������
				 );

// �� 1 ά����������������������������
void Sort_byIndex_double(double * p,	// ��������������������飬����ִ����������е�Ԫ�ؽ��� idx �е�����������������
				         int n,			// ���룺���鳤�ȣ�Ԫ�ظ���
				         int * idx		// ���룺����ֵ���������Ǹ���������������е��������������еĸ�Ԫ�ؽ��������еģ������� 0 ��ʼ
				         );

// �� 1 ά����������������������������
void Sort_byIndex_int(int * p,			// ��������������������飬����ִ����������е�Ԫ�ؽ��� idx �е�����������������
				      int n,			// ���룺���鳤�ȣ�Ԫ�ظ���
				      int * idx			// ���룺����ֵ���������Ǹ���������������е��������������еĸ�Ԫ�ؽ��������еģ������� 0 ��ʼ
				      );

// �� 2 ά�����ÿһ�а�ĳһ�е����ݽ���������߽�������
void Sort2_byRow(double ** p,			// ����������������� 2 ά����
				 int w, int h,			// ���룺2 ά����Ŀ���
				 int row = 0,			// ���룺��ĳһ�е����ݶ������н�������Ĭ��Ϊ�� 1 �У������� 0 ��ʼ
				 BOOL bIsAscent = TRUE	// ���룺�������ǽ������У�Ĭ��Ϊ����
				 );

// �� 2 ά�����ÿһ�а�ĳһ�е����ݽ���������߽�������
void Sort2_byCol(double ** p,			// ����������������� 2 ά����
				 int w, int h,			// ���룺2 ά����Ŀ���
				 int col = 0,			// ���룺��ĳһ�е����ݶ������н�������Ĭ��Ϊ�� 1 �У������� 0 ��ʼ
				 BOOL bIsAscent = TRUE	// ���룺�������ǽ������У�Ĭ��Ϊ����
				 );

// �ҵ������� 1 ά�����е���Сֵ���������Сֵ�������е�����
double FindArrayMin(double * p,					// ���룺������� 1 ά����
					int n,						// ���룺����ĳ���
					int & idx,					// ������ҵ���������Сֵ���������� 0 ��ʼ
					double * pMinThresh = NULL	// even the minimal value is supposed to be larger than this threshold
					);

// �ҵ������� 1 ά�����е����ֵ����������ֵ�������е�����
double FindArrayMax(double * p,	// ���룺������� 1 ά����
					int n,		// ���룺����ĳ���
					int & idx	// ������ҵ����������ֵ���������� 0 ��ʼ
					);

// �ҵ������� 1 ά�����е����ֵ����������ֵ�������е�����
BYTE FindArrayMax_BYTE(BYTE * p,	// ���룺������� 1 ά����
					   int n,		// ���룺����ĳ���
					   int & idx	// ������ҵ����������ֵ���������� 0 ��ʼ
					   );

// �ҵ������� 1 ά�����е����ֵ����������ֵ�������е�����
int FindArrayMax_int(int * p,		// ���룺������� 1 ά����
					 int n,			// ���룺����ĳ���
					 int & idx		// ������ҵ����������ֵ���������� 0 ��ʼ
					 );

// ���� double �͵ı���������ֵ
void Swap_double(double & A, double & B);
// ���� int �͵ı���������ֵ
void Swap_int(int & A, int & B);

// The same procedure with drawing a line, except that this function output
// the points on the line, instead of actually drawing a line.
void GetLinePts(int x0, int y0,			// input: integers, the coordinates of the starting point
				int x1, int y1,			// input: integers, the coordinates of the ending point
				int * px, int * py,		// output:the generated n points on the line
				int & n					// output:the amount of points in the output
				);

void SaveCameraData(CString path, const cam_data & cam);
void ReadCameraData(CString path, cam_data & cam);

void DetermineInterval(double val_max, double val_min, double val_cur, double radius, double & inter_max, double & inter_min);

// compute exp((-1/2)*(x-miu)^2/sigma^2)
double exp_miu_sigma(double x, double miu, double sigma);

}