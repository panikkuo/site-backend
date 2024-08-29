#include <jwt-cpp/jwt.h>
#include <fstream>
#include <string>

class JwtFunctions {
private:
    std::string secretKey;
public:
    void SetToken(const std::string secretKeyFileName) {
        std::ifstream secretKeyFile;
        secretKeyFile.open(secretKeyFileName);
        secretKeyFile >> secretKey;
        std::cout << "Secret key is added" << std::endl;
        secretKeyFile.close();
    }
    std::string GetToken(const std::string& login) {
        auto token = jwt::create()
            .set_issuer("auth0")
            .set_type("JWT")
            .set_payload_claim("login", jwt::claim(std::string{ login }))
            .sign(jwt::algorithm::hs256{ secretKey });

        return token;
    }
    bool CheckUserLoginToken(const std::string token, const std::string login) {
        auto decodedToken = jwt::decode(token);
        
        if (!decodedToken.has_payload_claim("login"))
            return false;

        std::string tokenLogin = decodedToken.get_payload_claim("login").as_string();

        if (tokenLogin == login)
            return true;
        else
            return false;
    }
};