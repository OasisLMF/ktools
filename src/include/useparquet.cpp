#include "useparquet.h"


void OpenParquetFile(std::shared_ptr<arrow::io::FileOutputStream>& outFile,
		     const std::string& fileName) {

  auto status_name = arrow::io::FileOutputStream::Open(fileName);
  ::arrow::Status s = ::arrow::internal::GenericToStatus(status_name.status());
  if (!s.ok()) {
    throw ::parquet::ParquetStatusException(std::move(s));
  }

  outFile = std::move(status_name).ValueOrDie();

}


parquet::StreamWriter SetupParquetOutputStream(const std::string& fileName,
	const std::vector<ParquetFields>& parquetFields) {

  std::shared_ptr<arrow::io::FileOutputStream> outFile;
  OpenParquetFile(outFile, fileName);

  parquet::WriterProperties::Builder builder;
  std::shared_ptr<parquet::schema::GroupNode> schema;
  parquet::schema::NodeVector fields;

  for (std::vector<ParquetFields>::const_iterator it = parquetFields.begin();
       it != parquetFields.end(); ++it) {
    fields.push_back(parquet::schema::PrimitiveNode::Make(it->name,
							  parquet::Repetition::REQUIRED,
							  it->type, it->converted_type));
  }

  schema = std::static_pointer_cast<parquet::schema::GroupNode>(
    parquet::schema::GroupNode::Make("schema", parquet::Repetition::REQUIRED, fields));

  parquet::StreamWriter os{parquet::ParquetFileWriter::Open(
    outFile, schema, builder.build())};

  return os;

}
