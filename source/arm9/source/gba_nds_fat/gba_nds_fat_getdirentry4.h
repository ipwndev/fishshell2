        u16 *pcs[13]={&lfn.char0,&lfn.char1,&lfn.char2,&lfn.char3,&lfn.char4,&lfn.char5,&lfn.char6,&lfn.char7,&lfn.char8,&lfn.char9,&lfn.char10,&lfn.char11,&lfn.char12};
        
        // rawdata(ansi)
/*
        for(u32 idx=0;idx<13;idx++){
          u8 *pc=(u8*)pcs[idx];
          lfnName[lfnPos + idx] = pc[0];
        }
*/
        
        // rawdata(Unicode)
        
        for(u32 idx=0;idx<13;idx++){
          u8 *pc=(u8*)pcs[idx];
          u16 uc;
          uc=pc[0] | (pc[1]<<8);
          lfnNameUnicode[lfnPos + idx] = uc;
        }
