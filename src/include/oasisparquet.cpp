#include "oasisparquet.h"


void OasisParquet::OpenParquetFile(std::shared_ptr<arrow::io::FileOutputStream>& outFile,
		     const std::string& fileName) {

  auto status_name = arrow::io::FileOutputStream::Open(fileName);
  ::arrow::Status s = ::arrow::internal::GenericToStatus(status_name.status());
  if (!s.ok()) {
    throw ::parquet::ParquetStatusException(std::move(s));
  }

  outFile = std::move(status_name).ValueOrDie();

}


void OasisParquet::OpenParquetFile(std::shared_ptr<arrow::io::ReadableFile>& inFile,
		     const std::string& fileName) {

  auto status_name = arrow::io::ReadableFile::Open(fileName);
  ::arrow::Status s = ::arrow::internal::GenericToStatus(status_name.status());
  if (!s.ok()) {
    throw ::parquet::ParquetStatusException(std::move(s));
  }

  inFile = std::move(status_name).ValueOrDie();

}


parquet::StreamReader OasisParquet::SetupParquetInputStream(const std::string& fileName) {

  std::shared_ptr<arrow::io::ReadableFile> inFile;
  OpenParquetFile(inFile, fileName);

  parquet::StreamReader os{parquet::ParquetFileReader::Open(inFile)};

  return os;

}


parquet::StreamWriter OasisParquet::SetupParquetOutputStream(const std::string& fileName,
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

parquet::StreamWriter OasisParquet::GetParquetStreamWriter(const int ordTableName,
					     const std::string outFile) {

  std::vector<ParquetFields> parquetFields;

  // Period Loss Tables
  if (ordTableName == MPLT || ordTableName == QPLT || ordTableName == SPLT) {

    parquetFields.push_back({"Period", parquet::Type::INT32,
			    parquet::ConvertedType::INT_32});
    parquetFields.push_back({"PeriodWeight", parquet::Type::FLOAT,
			    parquet::ConvertedType::NONE});
    parquetFields.push_back({"EventId", parquet::Type::INT32,
			    parquet::ConvertedType::INT_32});
    parquetFields.push_back({"Year", parquet::Type::INT32,
			    parquet::ConvertedType::INT_32});
    parquetFields.push_back({"Month", parquet::Type::INT32,
			    parquet::ConvertedType::INT_32});
    parquetFields.push_back({"Day", parquet::Type::INT32,
			    parquet::ConvertedType::INT_32});
    parquetFields.push_back({"Hour", parquet::Type::INT32,
			    parquet::ConvertedType::INT_32});
    parquetFields.push_back({"Minute", parquet::Type::INT32,
			    parquet::ConvertedType::INT_32});
    parquetFields.push_back({"SummaryId", parquet::Type::INT32,
			    parquet::ConvertedType::INT_32});
    if (ordTableName == MPLT) {
      parquetFields.push_back({"SampleType", parquet::Type::INT32,
			      parquet::ConvertedType::INT_32});
      parquetFields.push_back({"ChanceOfLoss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"MeanLoss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"SDLoss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"MaxLoss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"FootprintExposure", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"MeanImpactedExposure", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"MaxImpactedExposure", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
    } else if (ordTableName == SPLT) {
      parquetFields.push_back({"SampleId", parquet::Type::INT32,
			      parquet::ConvertedType::INT_32});
      parquetFields.push_back({"Loss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"ImpactedExposure", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
    } else if (ordTableName == QPLT) {
      parquetFields.push_back({"Quantile", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"Loss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
    }

  // Event Loss Tables
  } else if (ordTableName == MELT || ordTableName == QELT || ordTableName == SELT) {

    parquetFields.push_back({"EventId", parquet::Type::INT32,
			    parquet::ConvertedType::INT_32});
    parquetFields.push_back({"SummaryId", parquet::Type::INT32,
			    parquet::ConvertedType::INT_32});
    if (ordTableName == MELT) {
      parquetFields.push_back({"SampleType", parquet::Type::INT32,
			      parquet::ConvertedType::INT_32});
      parquetFields.push_back({"EventRate", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"ChanceOfLoss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"MeanLoss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"SDLoss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"MaxLoss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"FootprintExposure", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"MeanImpactedExposure", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"MaxImpactedExposure", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
    } else if (ordTableName == QELT) {
      parquetFields.push_back({"Quantile", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"Loss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
    } else if (ordTableName == SELT) {
      parquetFields.push_back({"SampleId", parquet::Type::INT32,
			      parquet::ConvertedType::INT_32});
      parquetFields.push_back({"Loss", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
      parquetFields.push_back({"ImpactedExposure", parquet::Type::FLOAT,
			      parquet::ConvertedType::NONE});
    }

  } else if (ordTableName == NONE) {

    fprintf(stderr, "FATAL: No table type selected - please select table "
		      "type for files to be concatenated.\n");
    exit(EXIT_FAILURE);

  } else {   // Should not get here
    fprintf(stderr, "FATAL: Unrecognised parquet table name %d\n",
	    ordTableName);
    exit(EXIT_FAILURE);
  }

  parquet::StreamWriter os = SetupParquetOutputStream(outFile, parquetFields);

  return os;

}
