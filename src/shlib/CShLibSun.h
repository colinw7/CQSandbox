class CShLibSun : public CShLibImpl {
 public:
  CShLibSun(const std::string &filename);
 ~CShLibSun();

  bool open() override;
  bool getProc(const std::string &name, CShLibProc *proc) override;
  bool getData(const std::string &name, CShLibData *data) override;
  void close() override;
};
