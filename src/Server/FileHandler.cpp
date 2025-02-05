#include "../../inc/Server/FileHandler.hpp"

FileHandler::FileHandler() {
  current_dir = fs::current_path();
}
void FileHandler::ChangeDirectory(int client_fd, const std::string& new_dir){
  new_path = current_dir / new_dir;
  if (fs::exists(new_path) && fs::is_directory(new_path)) {
    current_dir = new_path;
    std::cout << current_dir << std::endl;
    SendResponse(client_fd, "Directory changed to " + current_dir.string() + "\r\n");
  } else {
    SendResponse(client_fd, "Failed to change directory\r\n");
  }
}
void FileHandler::SendResponse(int client_fd, const std::string& response) {
  send(client_fd, response.c_str(), response.size(), 0);
}
void FileHandler::ListFiles(int client_fd) {
    std::string file_list = "Here comes the directory listing\r\n";
    std::cout << current_dir << std::endl;
    for (const auto& entry : fs::directory_iterator(current_dir)) {
        std::string entry_name = entry.path().filename().string();

        if (fs::is_directory(entry)) {
            // Directories in blue
            file_list += "\033[34m" + entry_name + "\033[0m\r\n";
        } else if (fs::is_regular_file(entry)) {
            // Regular files in green
            file_list += "\033[32m" + entry_name + "\033[0m\r\n";
        } else {
            // Other types (e.g., symlinks) in default color
            file_list += entry_name + "\r\n";
        }
    }

    file_list += "Directory send OK\r\n";
    SendResponse(client_fd, file_list);
}
bool FileHandler::OpenFile(std::fstream& file, fs::path filepath, std::ios_base::openmode mode, int client_fd) {
    file.open(filepath, mode);
    if (!file.is_open()) {
      SendResponse(client_fd, "Failed to open file\r\n");
        return false;
    }
    return true;
}

bool FileHandler::fileExists(const std::string& filepath) {
    return std::filesystem::exists(filepath);
}

bool FileHandler::uPLOAD(int client_fd, const std::string& fileName) {
  fs::path file_path = current_dir / fileName;
  std::fstream file;
  if (!OpenFile(file, file_path, std::ios::binary | std::ios::out, client_fd)) {
    SendResponse(client_fd, "Failed to create file\r\n");
    std::cerr << "Failed to create file\n";
    return false;
  }

    char buffer[BUFF_SIZE];
    size_t totalBytes = 0;

    while (true) {
        int valread = recv(client_fd, buffer, BUFF_SIZE, 0);
        if (valread < 0) {
            send(client_fd, "ERROR: File transfer failed\n", 27, 0);
            file.close();
            return false;
        } else if (valread == 0) {
            break; // End of file
        }
        file.write(buffer, valread);
        totalBytes += valread;
    }

    file.close();
    send(client_fd, "SUCCESS: File uploaded\n", 23, 0);
    return true;
}

bool FileHandler::dOWNLOAD(int client_fd, const std::string& fileName) {
  fs::path file_path = current_dir / fileName;
  if (!fs::exists(file_path)) {
    SendResponse(client_fd, "File not found\r\n");
    return false;
  }
  std::fstream file;
  if (!OpenFile(file, file_path, std::ios::binary | std::ios::in, client_fd)) {
    SendResponse(client_fd, "Failed to open file\r\n");
    std::cerr << "ERROR: Unable to open file\n";
    return false;
  }
  char buffer[BUFF_SIZE];
  size_t totalBytes = 0;
  while (!file.eof()) {
    file.read(buffer, sizeof(buffer));
    int valread = file.gcount();
    if (valread > 0) {
      if (send(client_fd, buffer, valread, 0) < 0) {
        file.close();
        return false;
      }
      totalBytes += valread;
    }
  }
  file.close();
  send(client_fd, "SUCCESS: File downloaded\n", 25, 0);
  return true;
}
fs::path FileHandler::GetCurrent_dir() const
{
  return (current_dir);
}
