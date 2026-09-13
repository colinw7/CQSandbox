class CShLibHP : public CShLibImpl {
 public:
  CShLibHP(const std::string &filename);
 ~CShLibHP();

  bool open() override;
  bool getProc(const std::string &name, CShLibProc *proc) override;
  bool getData(const std::string &name, CShLibData *data) override;
  void close() override;
};
