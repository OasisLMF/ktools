#ifndef USEPARQUET_H_
#define USEPARQUET_H_

#include "arrow/io/file.h"
#include "parquet/exception.h"
#include "parquet/stream_reader.h"
#include "parquet/stream_writer.h"


struct ParquetFields {
  std::string name;
  parquet::Type::type type;
  parquet::ConvertedType::type converted_type;
};

void OpenParquetFile(std::shared_ptr<arrow::io::FileOutputStream>& outFile,
		     const std::string& fileName);
parquet::StreamWriter SetupParquetOutputStream(const std::string& fileName,
  const std::vector<ParquetFields>& parquetFields);

#endif
