//Основные дейстивия: 
//понять по фоткам кто где 1) Дописать админский интерфейс !
//C2) Дописать интерефейс во вкладке логин !! проверить
//С3) Разобраться с jwt (Все сделано надо понять как все проверять) + дописать !!! Проверить
//Спросить у владоса 4) Добавить блюдей в меню и разобраться с изображением блюда из яндекс диска !!!
//С5) Заменить получение броней(сперва дата потом дается время и на это скидывать свободные столы) !!! Проверить
//hzkrya mb norm napisano 6) В некоторых местах особенно в HtppHandlerFunctions переписать монолитные функции под маленькие

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <drogon/HttpFilter.h>

#include "Objects.h"
#include "DataBaseMainFunctions.h"
#include "HttpHanlderFunctions.h"

std::shared_ptr<DataBaseFunctions> DataBaseConnection;
std::shared_ptr<HandleRequestFunctions> handleRequest;

void SetMethods() {
	drogon::app().registerHandler(
		"/api/v1/register",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
				handleRequest->HandlePostRegisterUser(request, std::move(callback), DataBaseConnection);
		},
		{ drogon::Post, drogon::Options }
	);
	drogon::app().registerHandler(
		"/api/v1/menu",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
				handleRequest->HandleGetMenu(request, std::move(callback), DataBaseConnection);
		},
		{ drogon::Get }
		);
	drogon::app().registerHandler(
		"/api/v1/authorization",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
				handleRequest->HandlePostAuthorization(request, std::move(callback), DataBaseConnection);
		},
		{ drogon::Post, drogon::Options }
	);
	drogon::app().registerHandler(
		"/api/v1/login/{1}",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string userLogin) {
				handleRequest->HandleGetDeleteLogin(request, std::move(callback), DataBaseConnection, userLogin);
		},
		{ drogon::Get, drogon::Delete, drogon::Options }
	);
	drogon::app().registerHandler(
		"/api/v1/login/{1}/reservation",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string userLogin) {
				handleRequest->HandleGetUserReservation(request, std::move(callback), DataBaseConnection, userLogin);
		},
		{ drogon::Get, drogon::Options }
		);
	drogon::app().registerHandler(
		"/api/v1/login/{1}/reservation/{2}",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string userLogin, const Id id) {
				handleRequest->HandleDeleteUserReservation(request, std::move(callback), DataBaseConnection, userLogin, id);
		},
		{ drogon::Get, drogon::Options }
		);
	drogon::app().registerHandler(
		"/api/v1/reservation/{1}/{2}",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string date, const std::string time) {
				handleRequest->HandleGetReservation(request, std::move(callback), DataBaseConnection, date, time);
		},
		{ drogon::Get }
	);
	drogon::app().registerHandler(
		"/api/v1/reservation",
		[](const drogon::HttpRequestPtr& request,
			std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string date) {
				handleRequest->HandlePostReservation(request, std::move(callback), DataBaseConnection);
		},
		{ drogon::Post , drogon::Options}
		);
} 

int main() {

	setlocale(LC_ALL, "rus");

	try {
		/*std::string dbname, user, password, host, port;
		std::cout << "Data base name (use restaurant): "; std::cin >> dbname;
		std::cout << "User name (defual postgres) : "; std::cin >> user;
		std::cout << "Password : "; std::cin >> password;
		std::cout << "host : "; std::cin >> host;
		std::cout << "port (default 5432) : "; std::cin >> port;*/
		DataBaseConnection = std::make_shared<DataBaseFunctions>("restaurant", "postgres", "nikanika228", "localhost", "5432");
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}

	handleRequest = std::make_shared<HandleRequestFunctions>();
	std::string fileName;
	std::cout << "secret key file : ";  std::cin >> fileName; 
	fileName = "key.txt";
	handleRequest->SetToken(fileName);

	SetMethods();

	std::string ip;
	/*std::cout << "set backend ip : "; std::cin >> ip;*/
	ip = "26.33.89.53";
	std::cout << "Server is started! " << std::endl;
	drogon::app().addListener(ip, 8080).run();

	return 0;
}