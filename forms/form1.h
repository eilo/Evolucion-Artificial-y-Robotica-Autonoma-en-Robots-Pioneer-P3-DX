#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace forms {

	/// <summary>
	/// Resumen de form1
	///
	/// ADVERTENCIA: si cambia el nombre de esta clase, deberá cambiar la
	///          propiedad 'Nombre de archivos de recursos' de la herramienta de compilación de recursos administrados
	///          asociada con todos los archivos .resx de los que depende esta clase. De lo contrario,
	///          los diseñadores no podrán interactuar correctamente con los
	///          recursos adaptados asociados con este formulario.
	/// </summary>
	public ref class form1 : public System::Windows::Forms::Form
	{
	public:
		form1(void)
		{
			InitializeComponent();
			//
			//TODO: agregar código de constructor aquí
			//
		}

	protected:
		/// <summary>
		/// Limpiar los recursos que se estén utilizando.
		/// </summary>
		~form1()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		/// <summary>
		/// Variable del diseñador requerida.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Método necesario para admitir el Diseñador. No se puede modificar
		/// el contenido del método con el editor de código.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = gcnew System::ComponentModel::Container();
			this->Size = System::Drawing::Size(300,300);
			this->Text = L"form1";
			this->Padding = System::Windows::Forms::Padding(0);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
        }
#pragma endregion
	};
}
