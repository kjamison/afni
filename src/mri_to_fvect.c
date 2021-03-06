#include "mrilib.h"

/*-------------------------------------------------------------------*/
/*! Convert an image of vectors to an array of single-valued images. */

MRI_IMARR * mri_fvect_to_imarr( MRI_IMAGE *inim )
{
   float *var ;
   MRI_IMAGE *aim ; float *aar ; MRI_IMARR *outar ;
   int nvox , vd , kk , ii ;

ENTRY("mri_fvect_to_imarr") ;

   if( inim == NULL || inim->kind != MRI_fvect ) RETURN(NULL) ;

   var = mri_data_pointer(inim) ; if( var == NULL ) RETURN(NULL) ;

   vd   = inim->vdim ; if( vd <= 0 ) RETURN(NULL) ;
   nvox = inim->nvox ;

   INIT_IMARR(outar) ;

   for( kk=0 ; kk < vd ; kk++ ){
     aim = mri_new_conforming( inim , MRI_float ) ;
     aar = aim->im ;
     for( ii=0 ; ii < nvox ; ii++ ) aar[ii] = var[ii*vd+kk] ;
     MRI_COPY_AUX(aim,inim) ; ADDTO_IMARR(outar,aim) ;
   }

   RETURN(outar) ;
}

/*-------------------------------------------------------------------*/
/*! Extract 1 float sub-image from an fvect image. [09 Dec 2008] */

MRI_IMAGE * mri_fvect_subimage( MRI_IMAGE *inim , int kk )
{
   float *var ;
   MRI_IMAGE *aim ; float *aar ;
   int nvox , vd , ii ;

ENTRY("mri_fvect_to_subimage") ;

   if( inim == NULL || inim->kind != MRI_fvect ) RETURN(NULL) ;
   if( kk < 0 || kk >= inim->vdim ) RETURN(NULL) ;

   var = mri_data_pointer(inim) ; if( var == NULL ) RETURN(NULL) ;

   vd   = inim->vdim ; if( vd <= 0 ) RETURN(NULL) ;
   nvox = inim->nvox ;

   aim = mri_new_conforming( inim , MRI_float ) ;
   aar = MRI_FLOAT_PTR(aim) ;
   for( ii=0 ; ii < nvox ; ii++ ) aar[ii] = var[ii*vd+kk] ;
   MRI_COPY_AUX(aim,inim) ;
   RETURN(aim) ;
}

/*-------------------------------------------------------------------*/

MRI_IMAGE * mri_imarr_to_fvect( MRI_IMARR *imar )
{
   MRI_IMAGE *vim , *aim ;
   float     *var , *aar ;
   int vd , nvox , kk , ii ;

ENTRY("mri_imarr_to_fvect") ;

   if( imar == NULL || IMARR_COUNT(imar) < 1 ) RETURN(NULL) ;

   vd   = IMARR_COUNT(imar) ;
   aim  = IMARR_SUBIM(imar,0) ;
   nvox = aim->nvox ;
   vim  = mri_empty_conforming( aim , MRI_fvect ) ;
   mri_adjust_fvectim( vim , vd ) ; MRI_COPY_AUX(vim,aim) ;
   var = (float *)vim->im ;

   for( kk=0 ; kk < vd ; kk++ ){
     aim = IMARR_SUBIM(imar,kk) ;
     if( aim->nvox < nvox ) continue ;
     if( aim->kind != MRI_float ) aim = mri_to_float(aim) ;
     aar = (float *)aim->im ;
     for( ii=0 ; ii < nvox ; ii++ ) var[ii*vd+kk] = aar[ii] ;

     if( aim != IMARR_SUBIM(imar,kk) ) mri_free(aim) ;
   }

   RETURN(vim) ;
}

/*-------------------------------------------------------------------*/

MRI_IMAGE * mri_pair_to_fvect( MRI_IMAGE *aim , MRI_IMAGE *bim )
{
   MRI_IMARR *imar ; MRI_IMAGE *vim ;

ENTRY("mri_pair_to_fvect") ;

   if( aim == NULL || bim == NULL ) RETURN(NULL) ;

   INIT_IMARR(imar) ; ADDTO_IMARR(imar,aim) ; ADDTO_IMARR(imar,bim) ;
   vim = mri_imarr_to_fvect( imar ) ;
   FREE_IMARR(imar) ;
   RETURN(vim) ;
}

/*-------------------------------------------------------------------*/

MRI_IMAGE * mri_triple_to_fvect( MRI_IMAGE *aim , MRI_IMAGE *bim , MRI_IMAGE *cim )
{
   MRI_IMARR *imar ; MRI_IMAGE *vim ;

ENTRY("mri_triple_to_fvect") ;

   if( aim == NULL || bim == NULL || cim == NULL ) RETURN(NULL) ;

   INIT_IMARR(imar) ;
   ADDTO_IMARR(imar,aim) ; ADDTO_IMARR(imar,bim) ; ADDTO_IMARR(imar,cim) ;
   vim = mri_imarr_to_fvect( imar ) ;
   FREE_IMARR(imar) ;
   RETURN(vim) ;
}

/*-------------------------------------------------------------------*/
/* 
   Turn an array of vectors into an MRI_IMAGE.
   Each vector k (vecs[k]) is a pointer to vec_len floats.
   k ranges from 0 to vec_num -1
*/
/*-------------------------------------------------------------------*/
MRI_IMAGE * mri_float_arrays_to_image(float **vecs, int vec_len, int vec_num)
{
   MRI_IMAGE *outim = NULL;
   int ii, jj, kk;
   float *tsar=NULL;
   
   ENTRY("mri_float_array_to_image");
   
   if (!vecs || vec_len <=0 || vec_num <= 0) RETURN(NULL); 
   
   if (!(tsar = (float *)calloc(vec_len*vec_num, sizeof(float)))) {
      ERROR_message("Failed to allocate for %d floats", vec_len*vec_num);
      RETURN(outim);
   }
   outim = mri_new_vol_empty( vec_len , vec_num , 1 , MRI_float ) ;
   kk=0;
   for (ii=0; ii<vec_num; ++ii) {
      for (jj=0; jj<vec_len; ++jj) {
         tsar[kk++] = vecs[ii][jj];
      }
   }
   mri_fix_data_pointer( tsar , outim ) ;
   mri_add_name( "IM_with_no_name" , outim ) ;
   
   RETURN(outim);
}


