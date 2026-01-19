


void swap(datafile_t file, datafile_t file2) {
    datafile_t tempfile;
    tempfile.data_Dump = file.data_Dump;
    tempfile.filename = file.filename;
    file.data_Dump = file2.data_Dump;
    file.filename = file2.filename;
    file2.data_Dump = tempfile.data_Dump;
    file2.filename = tempfile.filename;
}

void GuiCheats::sortchunk(datafile_t file, int i, int bufferSize) {
    u8 *buffer = new u8[bufferSize];
    file.data_Dump->getData(i, buffer, bufferSize);

    // sort them here
    u32 buffer_inc, data_inc;
    if (m_64bit_offset) {
        buffer_inc = sizeof(fromto_t);
        data_inc = sizeof(u64);
    } else {
        buffer_inc = sizeof(fromto32_t);
        data_inc = sizeof(u32);
    }
    if (m_64bit_offset) {
        u64 *to;
        to = (u64 *)(buffer);
        std::sort(to, to + bufferSize / data_inc);
    } else {
        u32 *to;
        to = (u32 *)(buffer);
        std::sort(to, to + bufferSize / data_inc);
    }

    file.data_Dump->putData(i, buffer, bufferSize);
	delete buffer;
};

void GuiCheats::mergechunk(datafile_t file, datafile_t file2, int sortedsize){
    file2.data_Dump->clear();
    int filesize = file.data_Dump->size();
	int bufferSize1 = MAX_BUFFER_SIZE;
	int bufferSize2 = MAX_BUFFER_SIZE;
	int sortedsize1 = sortedsize;
	int sortedsize2 = sortedsize;
    // merge them here
    for (int i = 0; i < filesize; i += sortedsize * 2) {
        int index1 = 0;
        int index2 = 0;
        int offset1 = 0;
        int offset2 = 0;
        if (sortedsize1 > filesize - i ) {
            sortedsize1 = filesize - i;
            sortedsize2 = 0;
        };
        if (sortedsize2 > filesize - i - sortedsize) {
            sortedsize2 = filesize - i - sortedsize;
        };
        if (bufferSize1 > sortedsize1) bufferSize1 = sortedsize1;
		if (bufferSize2 > sortedsize2) bufferSize2 = sortedsize2;

        u8 *buffer1 = new u8[bufferSize1];
        file.data_Dump->getData(i + offset1, buffer1, bufferSize1);

        u8 *buffer2 = nullptr;
        if (bufferSize2 != 0) {
            buffer2 = new u8[bufferSize2];
            file.data_Dump->getData(i + offset2 + sortedsize, buffer2, bufferSize2);
        }

        while (index1 + offset1 < sortedsize1 && index2 + offset2 < sortedsize2) {
            if (m_64bit_offset) {
                auto A = (u64 *)(buffer1 + index1);
                auto B = (u64 *)(buffer2 + index2);
                if (A < B) {
                    file2.data_Dump->addData((u8 *)&(A), 8);
                    index1 += 8;
                    if (index1 == bufferSize1) {
                        index1 = 0;
                        offset1 += bufferSize1;
                        if (bufferSize1 > sortedsize1 - offset1) bufferSize1 = sortedsize1 - offset1;
                        if (bufferSize1 > 0) file.data_Dump->getData(i + offset1, buffer1, bufferSize1);
                    }
                } else {
                    file2.data_Dump->addData((u8 *)&(B), 8);
                    index2 += 8;
                    if (index2 == bufferSize2) {
                        index2 = 0;
                        offset2 += bufferSize2;
                        if (bufferSize2 > sortedsize2 - offset2) bufferSize2 = sortedsize2 - offset2;
                        if (bufferSize2 > 0) file.data_Dump->getData(i + offset2 + sortedsize, buffer2, bufferSize2);
                    }
                }
            } else {
                auto A = (u32 *)(buffer1 + index1);
                auto B = (u32 *)(buffer2 + index2);
                if (A < B) {
                    file2.data_Dump->addData((u8 *)&(A), 4);
                    index1 += 4;
                    if (index1 == bufferSize1) {
                        index1 = 0;
                        offset1 += bufferSize1;
                        if (bufferSize1 > sortedsize1 - offset1) bufferSize1 = sortedsize1 - offset1;
                        if (bufferSize1 > 0) file.data_Dump->getData(i + offset1, buffer1, bufferSize1);
                    }					
                } else {
                    file2.data_Dump->addData((u8 *)&(B), 4);
                    index2 += 4;
                    if (index2 == bufferSize2) {
                        index2 = 0;
                        offset2 += bufferSize2;
                        if (bufferSize2 > sortedsize2 - offset2) bufferSize2 = sortedsize2 - offset2;
                        if (bufferSize2 > 0) file.data_Dump->getData(i + offset2 + sortedsize, buffer2, bufferSize2);
                    }					
                }
            }
        };
        while (index1 + offset1 < sortedsize1) {
            file2.data_Dump->addData((u8 *)&(buffer1[index1]), bufferSize1 - index1);
            index1 = 0;
            offset1 += bufferSize1;
            if (bufferSize1 > sortedsize1 - offset1) bufferSize1 = sortedsize1 - offset1;
            if (bufferSize1 > 0) file.data_Dump->getData(i + offset1, buffer1, bufferSize1);
        }
        while (index2 + offset2 < sortedsize2) {
            file2.data_Dump->addData((u8 *)&(buffer2[index2]), bufferSize2 - index2);
            index2 = 0;
            offset2 += bufferSize2;
            if (bufferSize2 > sortedsize2 - offset2) bufferSize2 = sortedsize2 - offset2;
            if (bufferSize2 > 0) file.data_Dump->getData(i + offset2 + sortedsize, buffer2, bufferSize2);
        }
        delete buffer1;
        if (buffer2 != nullptr) delete buffer2;
    }

        file2.data_Dump->flushBuffer();
};

void GuiCheats::sortfile(datafile_t file, datafile_t file2) {
    if (file.data_Dump == nullptr) {
        file.data_Dump = new MemoryDump(file.filename.c_str(), DumpType::DATA, false);
    };
    if (file2.data_Dump == nullptr) {
        file2.data_Dump = new MemoryDump(file2.filename.c_str(), DumpType::DATA, false);
    };

    int filesize = file.data_Dump->size();
    int bufferSize = MAX_BUFFER_SIZE; 
    int sortedsize = bufferSize;
    // sort the chunks first
    for (int i = 0; i < filesize; i += bufferSize) {
        if (bufferSize > filesize - i) {
            bufferSize = filesize - i;
        };
        sortchunk(file, i, bufferSize);
    }
    // Merge sort the chunks
    while (sortedsize < filesize) {
        mergechunk(file, file2, sortedsize);
        swap(file, file2);
        sortedsize *= 2;
    };
}
