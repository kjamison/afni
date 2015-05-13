/* KJ functions added for csa1 printing*/

/* print 'mesg' and then 'data' of length 'len' to 'fp'
 * if 'text': print as characters
 */
static int print_raw_data_kj(char * mesg, unsigned char * data, int len,
                          FILE * fp, int text)
{
   int mlen = 0, posn = 0, indent = 0, nbytes;

   if( mesg ) {
      mlen = strlen(mesg);
      fputs(mesg, fp);
   }

   while( posn < len ) {
      if( text && isprint(data[posn]) ) fputc(data[posn], fp);

      posn++;
   }

   return 0;
}

/* loop through all CSA data fields
 *
 * if name, only print that field contents
 * if data, print item data (1=raw, 2=text)
 * verb is verbose level
 *
 */
static int process_csa_data_kj(unsigned char * str, int len, int verb, int data)
{
   siemens_csa1_header * c1ptr = (siemens_csa1_header *)str;
   siemens_csa2_header * c2ptr = (siemens_csa2_header *)str;
   siemens_csa_tag     * ctag  = NULL;
   siemens_csa_item    * citem = NULL;
   unsigned char       * ucp   = (unsigned char *)str;  /* main pointer */
   unsigned int          itag, ntags;
   char                * endp;
   float                 stime = 0.0;
   int                   tagsize = sizeof(siemens_csa_tag);
   int                   posn, ctype, remain, nitem, iitem, ilen, ilenoff=-1;
   int                   little = little_endian();
   int 			 nbytesitem;
   FILE 	       * fp = stdout;

   if( !str ) {
      if(verb > 1) fprintf(stderr,"** PACSAD: bad data pointer\n");
      return 1;
   }

   if( verb > 2 ) {
      if( little ) fprintf(stderr,"-- little endian, so no swapping\n");
      else         fprintf(stderr,"-- big endian, will swap structures\n");
   }

   ctype = get_csa_type(ucp);
   posn = 0;  /* current index into ucp/str */
   if( ctype == 1 ) {
      if( ! little ) swap_csa1_header(c1ptr);
      if( verb > 1 ) print_csa1_header(c1ptr, stderr);
      posn += sizeof(siemens_csa1_header);
      ntags = c1ptr->ntags;
   } else {
      if( ! little ) swap_csa2_header(c2ptr);
      if( verb > 1 ) print_csa2_header(c2ptr, stderr);
      posn += sizeof(siemens_csa2_header);
      ntags = c2ptr->ntags;
   }

   if( ntags > 128 ) {
      if(verb > 1) fprintf(stderr,"** PACSAD: ntags (%d) > 128\n", ntags);
      return 1;
   }

   for( itag = 0; itag < ntags; itag++ ) {
      remain = len - posn;
      if( remain < tagsize ) {
         if(verb>1) fprintf(stderr,"** PACSAD: no room for next tag\n");
         return 1;
      }
      ctag = (siemens_csa_tag *)(ucp+posn);
      if( ! little ) swap_csa_tag(ctag);
      if( data || verb > 2 ) 
	fprintf(stdout,"\n%-64.64s = ", ctag->name);

      posn += sizeof(siemens_csa_tag);

      /* if CSA1 and FIRST tag, note the item length offset */
      if( ilenoff < 0 ) {
         if( ctype == 1 ) {
            ilenoff = ctag->nitems;
            if(verb > 2) fprintf(fp,"-- CSA1 item offset = %d\n",ilenoff);
            if( ilenoff < 0 ) ilenoff = 0;
         } else ilenoff = 0;
      }

      for( iitem = 0; iitem < ctag->nitems; iitem++ ) {
         citem = (siemens_csa_item *)(ucp+posn);
         if( ! little ) swap_csa_item(citem);
         nbytesitem=citem->xx[0];
         if( data || verb > 3 ){
            if( data && nbytesitem > 0 )
            	print_raw_data_kj(" ", citem->value, nbytesitem, fp, 1);
         }
         posn += csa_item_size(citem, ilenoff);
         if( posn > len ) break;
      }
   }
   fputc('\n',fp);
   return 0;
}

/* - search field '0x0029 1010' for string MosaicRefAcqTimes
 * - from there, find text formatted floats
 * - stop at AutoInlineImageFilterEnabled, if found
 */
static int print_siemens_csa1(PRV_ELEMENT_ITEM * elementItem)
{
   unsigned el_gr = DCM_TAG_GROUP(elementItem->element.tag);
   unsigned el_el = DCM_TAG_ELEMENT(elementItem->element.tag);
   int      el_len = elementItem->element.length;

   char * instr, * mstr;    /* input string and Mosaic string addr         */
   char * s2;               /* second search string, posn of AutoInline... */
   char * pstr;             /* position pointer, for reading times         */
   int    rem, rem2 = 0;    /* remainder counts                            */
   int    off, c, rv, diff; /* offset and counter vars                     */

   /* if no csa info requested, nothing to do */
   if ( !g_MDH_verb_csa1 ) return 0;

   /* if this is not the correct element, nothing to do */
   if( el_gr != 0x0029 || el_el != 0x1010 ) return 0;



   /* input string is field text, mstr is resulting MosaicRef text pointer */
   instr = (char *)elementItem->element.d.ot;

   process_csa_data_kj((unsigned char *)instr, el_len, g_MDH_verb_csa1,2);
   return 0;

}
/* END KJ functions */
