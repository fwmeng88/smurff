#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <cstdint>

#include <SmurffCpp/Configs/NoiseConfig.h>

namespace smurff
{
   class Data;
   class IDataWriter;
   class IDataCreator;

   class TensorConfig : public std::enable_shared_from_this<TensorConfig>
   {
   private:
      NoiseConfig m_noiseConfig;

   protected:
      bool m_isDense;
      bool m_isBinary;
      bool m_isScarce;

      std::uint64_t m_nmodes;
      std::uint64_t m_nnz;

      std::shared_ptr<std::vector<std::uint64_t> > m_dims;
      std::shared_ptr<std::vector<std::uint32_t> > m_columns;
      std::shared_ptr<std::vector<double> > m_values;

   private:
      std::string m_filename = "";

   protected:
      TensorConfig(bool isDense, bool isBinary, bool isScarce,
                   std::uint64_t nmodes, std::uint64_t nnz,
                   const NoiseConfig& noiseConfig);

   //
   // Dense double tensor constructors
   //
   public:
      TensorConfig(const std::vector<std::uint64_t>& dims, const std::vector<double> values,
                   const NoiseConfig& noiseConfig);

      TensorConfig(std::vector<std::uint64_t>&& dims, std::vector<double>&& values,
                   const NoiseConfig& noiseConfig);

      TensorConfig(std::shared_ptr<std::vector<std::uint64_t> > dims, std::shared_ptr<std::vector<double> > values,
                   const NoiseConfig& noiseConfig);

   //
   // Sparse double tensor constructors
   //
   public:
      TensorConfig(const std::vector<std::uint64_t>& dims, const std::vector<std::uint32_t>& columns, const std::vector<double>& values,
                   const NoiseConfig& noiseConfig, bool isScarce);

      TensorConfig(std::vector<std::uint64_t>&& dims, std::vector<std::uint32_t>&& columns, std::vector<double>&& values,
                   const NoiseConfig& noiseConfig, bool isScarce);

      TensorConfig(std::shared_ptr<std::vector<std::uint64_t> > dims, std::shared_ptr<std::vector<std::uint32_t> > columns, std::shared_ptr<std::vector<double> > values,
                   const NoiseConfig& noiseConfig, bool isScarce);

   //
   // Sparse binary tensor constructors
   //
   public:
      TensorConfig(const std::vector<std::uint64_t>& dims, const std::vector<std::uint32_t>& columns,
                   const NoiseConfig& noiseConfig, bool isScarce);

      TensorConfig(std::vector<std::uint64_t>&& dims, std::vector<std::uint32_t>&& columns,
                   const NoiseConfig& noiseConfig, bool isScarce);

      TensorConfig(std::shared_ptr<std::vector<std::uint64_t> > dims, std::shared_ptr<std::vector<std::uint32_t> > columns,
                   const NoiseConfig& noiseConfig, bool isScarce);

   public:
      virtual ~TensorConfig();

   public:
      const NoiseConfig& getNoiseConfig() const;
      void setNoiseConfig(const NoiseConfig& value);

      bool isDense() const;
      bool isBinary() const;
      bool isScarce() const;

      std::uint64_t getNModes() const;
      std::uint64_t getNNZ() const;

      const std::vector<std::uint64_t>& getDims() const;
      const std::vector<std::uint32_t>& getColumns() const;
      const std::vector<double>& getValues() const;

      std::shared_ptr<std::vector<std::uint64_t> > getDimsPtr() const;
      std::shared_ptr<std::vector<std::uint32_t> > getColumnsPtr() const;
      std::shared_ptr<std::vector<double> > getValuesPtr() const;

      void setFilename(const std::string &f);
      const std::string &getFilename() const;

   public:
      virtual std::ostream& info(std::ostream& os) const;
      virtual std::ostream& save(std::ostream& os) const;

   public:
      virtual std::shared_ptr<Data> create(std::shared_ptr<IDataCreator> creator) const;

   public:
      virtual void write(std::shared_ptr<IDataWriter> writer) const;
   };
}
