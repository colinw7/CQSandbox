class CShLibSGI : public CShLibImpl {
 public:
  CShLibSGI(const std::string &filename);
 ~CShLibSGI();

  bool open() override;
  bool getProc(const std::string &name, CShLibProc *proc) override;
  bool getData(const std::string &name, CShLibData *data) override;
  void close() override;
};
