#ifndef USEPARQUET_H_
#define USEPARQUET_H_

#include "oasis.h"

#include "arrow/io/file.h"
#include "parquet/exception.h"
#include "parquet/stream_writer.h"


#ifdef OASIS_FLOAT_TYPE_DOUBLE
const parquet::Type:type OASIS_PARQUET_FLOAT = parquet::Type::DOUBLE;
#else
const parquet::Type::type OASIS_PARQUET_FLOAT = parquet::Type::FLOAT;
#endif

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
