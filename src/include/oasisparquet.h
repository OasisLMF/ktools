#ifndef OASISPARQUET_H_
#define OASISPARQUET_H_

#include "oasis.h"

#include "arrow/io/file.h"
#include "parquet/exception.h"
#include "parquet/stream_reader.h"
#include "parquet/stream_writer.h"


namespace OasisParquet {

  enum { NONE = 0, MPLT, QPLT, SPLT, MELT, QELT, SELT };

  struct ParquetFields {
    std::string name;
    parquet::Type::type type;
    parquet::ConvertedType::type converted_type;
  };


  struct PLTEntry {
    int32_t period;
    float periodWeight;
    int32_t eventID;
    int32_t year;
    int32_t month;
    int32_t day;
    int32_t hour;
    int32_t minute;
    int32_t summaryID;
  };

  struct MomentPLTEntry : PLTEntry {
    int32_t sampleType;
    float chanceOfLoss;
    float meanLoss;
    float sdLoss;
    float maxLoss;
    float footprintExposure;
    float meanImpactedExposure;
    float maxImpactedExposure;

    friend parquet::StreamReader &operator>>(parquet::StreamReader &is,
					     MomentPLTEntry &row) {
      return is >> row.period >> row.periodWeight >> row.eventID >> row.year
		>> row.month >> row.day >> row.hour >> row.minute
		>> row.summaryID >> row.sampleType >> row.chanceOfLoss
		>> row.meanLoss >> row.sdLoss >> row.maxLoss
		>> row.footprintExposure >> row.meanImpactedExposure
		>> row.maxImpactedExposure >> parquet::EndRow;
    }

    friend parquet::StreamWriter &operator<<(parquet::StreamWriter &os,
					     const MomentPLTEntry &row) {
      return os << row.period << row.periodWeight << row.eventID << row.year
		<< row.month << row.day << row.hour << row.minute
		<< row.summaryID << row.sampleType << row.chanceOfLoss
		<< row.meanLoss << row.sdLoss << row.maxLoss
		<< row.footprintExposure << row.meanImpactedExposure
		<< row.maxImpactedExposure << parquet::EndRow;
    }
  };

  struct SamplePLTEntry : PLTEntry {
    int32_t sampleID;
    float loss;
    float impactedExposure;

    friend parquet::StreamReader &operator>>(parquet::StreamReader &is,
					     SamplePLTEntry &row) {
      return is >> row.period >> row.periodWeight >> row.eventID >> row.year
		>> row.month >> row.day >> row.hour >> row.minute
		>> row.summaryID >> row.sampleID >> row.loss
		>> row.impactedExposure >> parquet::EndRow;
    }

    friend parquet::StreamWriter &operator<<(parquet::StreamWriter &os,
					     const SamplePLTEntry &row) {
      return os << row.period << row.periodWeight << row.eventID << row.year
		<< row.month << row.day << row.hour << row.minute
		<< row.summaryID << row.sampleID << row.loss
		<< row.impactedExposure << parquet::EndRow;
    }
  };

  struct QuantilePLTEntry : PLTEntry {
    float quantile;
    float loss;

    friend parquet::StreamReader &operator>>(parquet::StreamReader &is,
					     QuantilePLTEntry &row) {
      return is >> row.period >> row.periodWeight >> row.eventID >> row.year
		>> row.month >> row.day >> row.hour >> row.minute
		>> row.summaryID >> row.quantile >> row.loss
		>> parquet::EndRow;
    }

    friend parquet::StreamWriter &operator<<(parquet::StreamWriter &os,
					     const QuantilePLTEntry &row) {
      return os << row.period << row.periodWeight << row.eventID << row.year
		<< row.month << row.day << row.hour << row.minute
		<< row.summaryID << row.quantile << row.loss
		<< parquet::EndRow;
    }
  };


  struct ELTEntry {
    int32_t eventID;
    int32_t summaryID;
  };

  struct MomentELTEntry : ELTEntry {
    int32_t sampleType;
    float eventRate;
    float chanceOfLoss;
    float meanLoss;
    float sdLoss;
    float maxLoss;
    float footprintExposure;
    float meanImpactedExposure;
    float maxImpactedExposure;

    friend parquet::StreamReader &operator>>(parquet::StreamReader &is,
					     MomentELTEntry &row) {
      return is >> row.eventID >> row.summaryID >> row.sampleType
		>> row.eventRate >> row.chanceOfLoss >> row.meanLoss
		>> row.sdLoss >> row.maxLoss >> row.footprintExposure
		>> row.meanImpactedExposure >> row.maxImpactedExposure
		>> parquet::EndRow;
    }

    friend parquet::StreamWriter &operator<<(parquet::StreamWriter &os,
					     const MomentELTEntry &row) {
      return os << row.eventID << row.summaryID << row.sampleType
		<< row.eventRate << row.chanceOfLoss << row.meanLoss
		<< row.sdLoss << row.maxLoss << row.footprintExposure
		<< row.meanImpactedExposure << row.maxImpactedExposure
		<< parquet::EndRow;
    }
  };

  struct QuantileELTEntry : ELTEntry {
    float quantile;
    float loss;

    friend parquet::StreamReader &operator>>(parquet::StreamReader &is,
					     QuantileELTEntry &row) {
      return is >> row.eventID >> row.summaryID >> row.quantile >> row.loss
		>> parquet::EndRow;
    }

    friend parquet::StreamWriter &operator<<(parquet::StreamWriter &os,
					     const QuantileELTEntry &row) {
      return os << row.eventID << row.summaryID << row.quantile << row.loss
		<< parquet::EndRow;
    }
  };

  struct SampleELTEntry : ELTEntry {
    int32_t sampleID;
    float loss;
    float impactedExposure;

    friend parquet::StreamReader &operator>>(parquet::StreamReader &is,
					     SampleELTEntry &row) {
      return is >> row.eventID >> row.summaryID >> row.sampleID >> row.loss
		>> row.impactedExposure >> parquet::EndRow;
    }

    friend parquet::StreamWriter &operator<<(parquet::StreamWriter &os,
					     const SampleELTEntry &row) {
      return os << row.eventID << row.summaryID << row.sampleID << row.loss
		<< row.impactedExposure << parquet::EndRow;
    }
  };


  void OpenParquetFile(std::shared_ptr<arrow::io::FileOutputStream>& outFile,
		       const std::string& fileName);
  void OpenParquetFile(std::shared_ptr<arrow::io::ReadableFile>& inFile,
		       const std::string& fileName);
  parquet::StreamReader SetupParquetInputStream(const std::string& fileName);
  parquet::StreamWriter SetupParquetOutputStream(const std::string& fileName,
	const std::vector<ParquetFields>& parquetFields);
  parquet::StreamWriter GetParquetStreamWriter(const int ordTableName,
					       const std::string outFile);
}
#endif
